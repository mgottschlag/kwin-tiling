=begin
/*
 *   Copyright 2008 Richard Dale <richard.j.dale@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
=end

require 'plasma_applet'
require 'cgi'
require 'rexml/document'
require 'soprano'

# Parser for SPARQL XML result set. Derived from the parser in the
# ActiveRDF SPARQL adapter code. Produces an Array of Hashes, each
# hash contains keys for each of the variables in the query, and
# values which are Soprano nodes.
#
class SparqlResultParser
  attr_reader :result

  def initialize
    @result = []
    @vars = []
    @current_type = nil
  end
  
  def tag_start(name, attrs)
    case name
    when 'variable'
      @vars << attrs['name']
    when 'result'
      @current_result = {}
    when 'binding'
      @current_binding = attrs['name']
    when 'bnode', 'uri'
      @current_type = name
    when 'literal', 'typed-literal'
      @current_type = name
      @datatype = attrs['datatype']
      @xmllang = attrs['xml:lang']
    end
  end
  
  def tag_end(name)
    if name == "result"
      @result << @current_result
    elsif name == 'bnode' || name == 'literal' || name == 'typed-literal' || name == 'uri'
      @current_type = nil
    elsif name == "sparql"
    end
  end
  
  def text(text)
    if !@current_type.nil?
      @current_result[@current_binding] = create_node(@current_type, @datatype, @xmllang, text)
    end
  end

  # create ruby objects for each RDF node
  def create_node(type, datatype, xmllang, value)
    case type
    when 'uri'
      Soprano::Node.new(Qt::Url.new(value))
    when 'bnode'
      Soprano::Node.new(value)
    when 'literal', 'typed-literal'
      if xmllang
        Soprano::Node.new(Soprano::LiteralValue.new(value), xmllang)
      elsif datatype
        Soprano::Node.new(Soprano::LiteralValue.fromString(value, Qt::Url.new(datatype)))
      else
        Soprano::Node.new(Soprano::LiteralValue.new(value))
      end
    end
  end
  
  def method_missing (*args)
  end
end

# There is a Soprano::Client::SparqlModel class in Soprano, but it
# appears to make queries synchronously and so it wouldn't be 
# suitable for a Plasma data engine like this one. Here we use KIO
# to retrieve the query results asynchronously.
#
class SparqlDataEngine < PlasmaScripting::DataEngine
  slots 'queryData(KIO::Job*, QByteArray)',
        'queryCompleted(KJob*)'

  def initialize(parent, args, endpoint, query, primary_value)
    super(parent)
    @endpoint = endpoint
    @query = query
    @primary_value = primary_value
  end

  def init
    setMinimumPollingInterval(120 * 1000)
  end

  def sourceRequestEvent(source_name)
    # puts "sourceRequested(#{source_name})"
    if @job
      return false
    end

    @source_name = source_name
    @sparql_results_xml = ""
    query_url = KDE::Url.new("#{@endpoint}?query=#{CGI.escape(@query % @source_name.gsub(' ', '_'))}")
    @job = KIO::get(query_url, KIO::Reload, KIO::HideProgressInfo)
    @job.addMetaData("accept", "application/sparql-results+xml" )
    connect(@job, SIGNAL('data(KIO::Job*, QByteArray)'), self,
            SLOT('queryData(KIO::Job*, QByteArray)'))
    connect(@job, SIGNAL('result(KJob*)'), self, SLOT('queryCompleted(KJob*)'))
    # setData(@source_name, {})
    return true
  end

  def queryData(job, data)
    # puts "queryData(#{job})"
    @sparql_results_xml += data.to_s
  end

  def queryCompleted(job)
    # puts "queryCompleted(#{job})"
    # puts @sparql_results_xml

    @job.doKill
    @job = nil
    parser = SparqlResultParser.new
    REXML::Document.parse_stream(@sparql_results_xml, parser)
    parser.result.each do |binding|
      binding.each_pair do |key, value|
        puts "#{key} --> #{value.inspect}"
        setData(binding[@primary_value].literal.variant.toString, key, value)
      end
    end
  end

  def updateSourceEvent(source_name)
    # puts "updateSource(#{source_name})"

    sourceRequestEvent(source_name)
    return true
  end
end

# In this query the properties are hard wired. It would be possible to
# make a query at start up to find all the properties for a given RDF
# type by retrieving the RDFS:domain properties specified in the 
# ontology:
#
# SELECT DISTINCT ?p WHERE { 
#     ?p <http://www.w3.org/2000/01/rdf-schema#domain> <http://mywebsite/myontology#MyClass> . 
# }
#
# Then dynamically construct a query like the one below with the properties.
#
SPARQL_QUERY = <<-EOS
PREFIX p: <http://dbpedia.org/property/>  
PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#>
SELECT * WHERE { 
     ?album p:artist  <http://dbpedia.org/resource/%s>.       
     ?album rdf:type <http://dbpedia.org/class/yago/Album106591815>.
     OPTIONAL {?album p:cover ?cover}.
     OPTIONAL {?album p:name ?name}.
     OPTIONAL {?album p:released ?dateofrelease}.
   }
EOS

module DbpediaAlbums
  #
  # Customize the use of the SparqlDataEngine by giving it the url of an endpoint,
  # a query to execute, and the name of the most important (or primary) value.
  # The '%s' in the query text above is replaced with the source name, with any
  # spaces replaced by underscores.
  #
  class Main < SparqlDataEngine
    def initialize(parent, args = nil)
      super(parent, args, 'http://dbpedia.org/sparql', SPARQL_QUERY, 'name')
    end
  end
end