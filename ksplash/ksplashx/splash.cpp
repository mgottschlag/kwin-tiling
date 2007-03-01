//#define DEBUG

const int MAX_ITEMS = 100;
const int LAST_STATE = 7;
const int ANIM_IMAGES_ROW = 10;


#include <config.h>

#include "splash.h"
#include "qcolor.h"
#include "qimage.h"
#include "pixmap.h"
#include "scale.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <dirent.h>

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif


struct AnimData
    {
    AnimData( int x, int y, PixmapData* frames, int num_frames, int delay );
    ~AnimData();
    bool updateFrame( int change );
    int x, y;
    PixmapData* frames;
    int num_frames;
    int current_frame;
    int delay;
    int remaining_delay;
    };

AnimData::AnimData( int x, int y, PixmapData* frames, int num_frames, int delay )
    : x( x ), y( y ), frames( frames ), num_frames( num_frames ), current_frame( 0 ), delay( delay ), remaining_delay( delay )
    {
    }

AnimData::~AnimData()
    {
    for( int i = 0;
         i < num_frames;
         ++i )
        {
        if( frames[ i ].hd != None )
            XFreePixmap( qt_xdisplay(), frames[ i ].hd );
        }
    delete[] frames;
    }

bool AnimData::updateFrame( int change )
    {
    remaining_delay -= change;
    bool ret = false;
    while( remaining_delay <= 0 )
        {
        if( ++current_frame == num_frames )
            current_frame = 0;
        remaining_delay += delay;
        ret = true;
        }
    return ret;
    }

static QImage splash_image; // contents of the splash window (needed for alphablending)
static Pixmap splash_pixmap; // the pixmap with window contents
static AnimData* animations[ MAX_ITEMS ];
static int anim_count;
static Window window = None;
static QRect geometry;
static bool scale_on = true;
static Atom kde_splash_progress;
static char theme_name[ 1024 ];
static char theme_dir[ 1024 ];
static bool test;
static int parent_pipe;
static time_t final_time;
static int state;
static time_t timestamp; // timestamp of the description.txt file, used for caching

// returns a pointer to a static !
static const char* findFileHelper( const char* name, int* w, int* h, bool locolor, bool lame )
    {
    static char tmp[ 1024 ];
    char best[ 1024 ];
    int best_w = -1;
    int best_h = -1;
    DIR* dir = opendir( theme_dir );
    if( dir != NULL )
        {
        while( dirent* file = readdir( dir ))
            {
            int w, h;
            if( locolor
                ? sscanf( file->d_name, "%dx%d-locolor", &w, &h ) == 2
                : sscanf( file->d_name, "%dx%d", &w, &h ) == 2 )
                {
                if( w > best_w
                    // only derive from themes with the same ratio if lame resolutions are not allowed, damn 1280x1024
                    && ( lame || w * screenGeometry().height() == h * screenGeometry().width())
                    )
                    {
                    snprintf( tmp, 1024, "%s/%dx%d%s/%s", theme_dir, w, h, locolor ? "-locolor" : "", name );
#ifdef DEBUG
                    fprintf( stderr, "FINDFILE3: %s %s\n", name, tmp );
#endif
                    if( access( tmp, R_OK ) == 0 )
                        {
                        best_w = w;
                        best_h = h;
                        strcpy( best, tmp );
                        }
                    }
                }
            }
        closedir( dir );
        }
    if( best_w > 0 )
        {
        if( w != NULL )
            *w = best_w;
        if( h != NULL )
            *h = best_h;
        strcpy( tmp, best );
        return tmp;
        }
    return "";
    }

// returns a pointer to a static !
static const char* findFileWithDepth( const char* name, int* w, int* h, bool locolor )
    {
    static char tmp[ 1024 ];
    snprintf( tmp, 1024, "%s/%dx%d%s/%s", theme_dir, screenGeometry().width(), screenGeometry().height(),
        locolor ? "-locolor" : "", name );
#ifdef DEBUG
    fprintf( stderr, "FINDFILE1: %s %s\n", name, tmp );
#endif
    if( access( tmp, R_OK ) != 0 )
        {
        // ksplashx/<theme>-<resolution>-<file> in 'kde-config --path cache'
        static char kdehome[ 1024 ];
        if( getenv( "KDEHOME" ) && getenv( "KDEHOME" )[ 0 ] )
            snprintf( kdehome, 1024, "%s", getenv( "KDEHOME" ));
        else
            snprintf( kdehome, 1024, "%s/.kde", getenv( "HOME" ) ? getenv( "HOME" ) : "" );
        static char hostname[ 1024 ];
        if( getenv("XAUTHLOCALHOSTNAME"))
            strncpy( hostname, getenv("XAUTHLOCALHOSTNAME"), 1023 );
        else
            gethostname( hostname, 1023 );
        hostname[ 1023 ] = '\0';
        snprintf( tmp, 1024, "%s/cache-%s/ksplashx/%s-%dx%d%s-%s", kdehome, hostname, theme_name,
            screenGeometry().width(), screenGeometry().height(), locolor ? "-locolor" : "", name );
#ifdef DEBUG
        fprintf( stderr, "FINDFILE2: %s %s\n", name, tmp );
#endif
        struct stat stat_buf;
        if( stat( tmp, &stat_buf ) != 0 || stat_buf.st_mtime < timestamp )
            {
            tmp[ 0 ] = '\0';
#ifdef DEBUG
            fprintf( stderr, "FINDFILE2 TIMESTAMP FAILURE\n" );
#endif
            }
        }
    if( access( tmp, R_OK ) == 0 )
        {
        if( w != NULL )
            *w = screenGeometry().width();
        if( h != NULL )
            *h = screenGeometry().height();
        return tmp;
        }
    if( w == NULL || h == NULL ) // no scaling possible
        return "";
    const char* ret = findFileHelper( name, w, h, locolor, false );
    if( ret == NULL || *ret == '\0' )
        ret = findFileHelper( name, w, h, locolor, true );
    return ret;
    }

// returns a pointer to a static !
static const char* findFile( const char* name, int* w = NULL, int* h = NULL, bool* locolor = NULL )
    {
    if( x11Depth() <= 8 )
        {
        if( const char* ret = findFileWithDepth( name, w, h, true )) // try locolor
            {
            if( locolor != NULL )
                *locolor = true;
            return ret;
            }
        }
    if( locolor != NULL )
        *locolor = false;
    return findFileWithDepth( name, w, h, false ); // no locolor
    }

// If a properly sized image doesn't exist save it in the cache location
// for the next use, because that means no scaling and a smaller png image
// to load.
static void pregeneratePixmap( const char* file, const char* real_file, int width, int height, bool locolor )
    {
#ifdef DEBUG
    static char cmd[ 1024 ];
    snprintf( cmd, 1024, "ksplashx_scale \"%s\" \"%s\" \"%s\" %d %d %d %d %ld %s", theme_name,
        file, real_file, width, height, screenGeometry().width(), screenGeometry().height(), timestamp,
        locolor ? "locolor" : "no-locolor" );
    fprintf( stderr, "PREGENERATE PIXMAP CMD:%s\n", cmd );
#endif
    char w[ 20 ], h[ 20 ], sw[ 20 ], sh[ 20 ], t[ 40 ];
    sprintf( w, "%d", width );
    sprintf( h, "%d", height );
    sprintf( sw, "%d", screenGeometry().width());
    sprintf( sh, "%d", screenGeometry().height());
    sprintf( t, "%ld", timestamp );
    if( fork() == 0 )
        {
        int maxf = sysconf( _SC_OPEN_MAX );
        for( int f = 0;
             f < maxf;
             ++f )
            close( f );
        nice( 10 );
        sleep( 30 );
        char* args[ 20 ];
        args[ 0 ] = "ksplashx_scale";
        args[ 1 ] = theme_name;
        args[ 2 ] = ( char* ) file;
        args[ 3 ] = ( char* ) real_file;
        args[ 4 ] = w;
        args[ 5 ] = h;
        args[ 6 ] = sw;
        args[ 7 ] = sh;
        args[ 8 ] = t;
        args[ 9 ] = ( char* )( locolor ? "locolor" : "no-locolor" );
        args[ 10 ] = NULL;
        execvp( args[ 0 ], args );
        _exit( 0 );
        }
    }

static QImage loadImage( const char* file )
    {
    int w, h;
    bool locolor;
    const char* real_file = findFile( file, &w, &h, &locolor ); // points to a static !
    FILE* f = fopen( real_file, "r" );
    if( f == NULL )
        return QImage();
    QImage img = splash_read_png_image( f );
    if( img.depth() != 32 )
        img = img.convertDepth( 32 );
    fclose( f );
    if( img.isNull())
        {
        fprintf( stderr, "Failed to load: %s\n", file );
        exit( 3 );
        }
    if( img.depth() != 32 )
        {
        fprintf( stderr, "Not 32bpp: %s\n", file );
        exit( 3 );
        }
    if( scale_on && ( w != screenGeometry().width() || h != screenGeometry().height()))
        {
        double ratiox = double( w ) / screenGeometry().width();
        double ratioy = double( h ) / screenGeometry().height();
#ifdef DEBUG
        fprintf( stderr, "PIXMAP SCALING: %f %f\n", ratiox, ratioy );
#endif
        img = scale( img, round( img.width() / ratiox ), round( img.height() / ratioy ));
        if( ratiox * ratioy > 1 ) // only downscale
            pregeneratePixmap( file, real_file, img.width(), img.height(), locolor );
        }
    return img;
    }

static void frameSize( const QImage& img, int frames, int& framew, int& frameh )
    {
    if( frames < ANIM_IMAGES_ROW )
        {
        framew = img.width() / frames;
        frameh = img.height();
        }
    else
        {
        framew = img.width() / ANIM_IMAGES_ROW;
        frameh = img.height() / (( frames + ANIM_IMAGES_ROW - 1 ) / ANIM_IMAGES_ROW );
        }
    }

static QImage loadAnimImage( const char* file, int frames )
    {
    int w, h;
    bool locolor;
    const char* real_file = findFile( file, &w, &h, &locolor ); // points to a static !
    FILE* f = fopen( real_file, "r" );
    if( f == NULL )
        {
        fprintf( stderr, "Bad anim file: %s\n", file );
        exit( 3 );
        }
    QImage img = splash_read_png_image( f );
    if( img.depth() != 32 )
        img = img.convertDepth( 32 );
    fclose( f );
    int framew, frameh;
    if( frames < ANIM_IMAGES_ROW )
        {
        if( img.width() % frames != 0 )
            {
            fprintf( stderr, "Bad anim size: %s\n", file );
            exit( 3 );
            }
        }
    else
        {
        if( img.width() % ANIM_IMAGES_ROW != 0
            || img.height() % (( frames + ANIM_IMAGES_ROW - 1 ) / ANIM_IMAGES_ROW ) != 0 )
            {
            fprintf( stderr, "Bad anim size: %s\n", file );
            exit( 3 );
            }
        }
    frameSize( img, frames, framew, frameh );
    if( scale_on && ( w != screenGeometry().width() || h != screenGeometry().height()))
        {
        double ratiox = double( w ) / screenGeometry().width();
        double ratioy = double( h ) / screenGeometry().height();
#ifdef DEBUG
        fprintf( stderr, "ANIM SCALING: %f %f\n", ratiox, ratioy );
#endif
        int framewnew = round( framew / ratiox );
        int framehnew = round( frameh / ratioy );
        QImage imgnew( framewnew * QMIN( frames, ANIM_IMAGES_ROW ),
            framehnew * (( frames + ANIM_IMAGES_ROW - 1 ) / ANIM_IMAGES_ROW ), img.depth());
        if( img.hasAlphaBuffer())
            imgnew.setAlphaBuffer( true );
        for( int frame = 0;
             frame < frames;
             ++frame )
            {
            QImage im2 = img.copy( ( frame % ANIM_IMAGES_ROW ) * framew, ( frame / ANIM_IMAGES_ROW ) * frameh, framew, frameh );
            im2 = scale( im2, framewnew, framehnew );
            // don't use bitBlt(), it'd apply also alpha
            for( int y = 0;
                 y < im2.height();
                 ++y )
                {
                QRgb* s = ( QRgb* ) im2.scanLine( y );
                QRgb* d = (( QRgb* ) imgnew.scanLine( y + ( frame / ANIM_IMAGES_ROW ) * framehnew ))
                    + ( frame % ANIM_IMAGES_ROW ) * framewnew;
                memcpy( d, s, im2.width() * sizeof( QRgb ));
                }
            }
        framew = framewnew;
        frameh = framehnew;
        img = imgnew;
        if( ratiox * ratioy > 1 ) // only downscale
            pregeneratePixmap( file, real_file, img.width(), img.height(), locolor );
        }
    return img;
    }

static PixmapData* imageAnimToPixmaps( const QImage& img, int frames )
    {
    if( img.isNull())
        return NULL;
    int framew, frameh;
    frameSize( img, frames, framew, frameh );
    PixmapData pix = imageToPixmap( img );
    PixmapData* ret = new PixmapData[ MAX_ITEMS ];
    GC gc = qt_xget_temp_gc( x11Screen(), false );
    for( int frame = 0;
         frame < frames;
         ++frame )
        {
        Pixmap p = XCreatePixmap( qt_xdisplay(), DefaultRootWindow( qt_xdisplay()), framew, frameh, x11Depth());
        XCopyArea( qt_xdisplay(), pix.hd, p, gc,
            ( frame % ANIM_IMAGES_ROW ) * framew, ( frame / ANIM_IMAGES_ROW ) * frameh, framew, frameh, 0, 0 );
        ret[ frame ].hd = p;
        ret[ frame ].w = framew;
        ret[ frame ].h = frameh;
        ret[ frame ].d = x11Depth();
        }
    if( pix.hd != None )
        XFreePixmap( qt_xdisplay(), pix.hd );
    return ret;
    }

static void doPaint( const QRect& area )
    {
#if 0
    fprintf( stderr, "PAINT: %d,%d-%dx%d\n", area.x(), area.y(), area.width(), area.height());
#endif
    if( window == None )
        return; // delayed
    // double-buffer
    Pixmap pixmap = XCreatePixmap( qt_xdisplay(), DefaultRootWindow( qt_xdisplay()),
        area.width(), area.height(), x11Depth());
    GC gc = qt_xget_temp_gc( x11Screen(), false );
    // copy splash pixmap
    XCopyArea( qt_xdisplay(), splash_pixmap, pixmap, gc,
        area.x(), area.y(), area.width(), area.height(), 0, 0 );
    // add animations
    for( int i = 0;
         i < MAX_ITEMS;
         ++i )
        {
        AnimData* anim = animations[ i ];
        PixmapData* frame = anim != NULL ? &anim->frames[ anim->current_frame ] : NULL;
        if( anim != NULL
            && area.intersects( QRect( anim->x, anim->y, frame->w, frame->h )))
            {
            XCopyArea( qt_xdisplay(), frame->hd, pixmap, gc,
                QMAX( 0, area.x() - anim->x ), QMAX( 0, area.y() - anim->y ),
                area.x() - anim->x + area.width(), area.y() - anim->y + area.height(),
                QMAX( 0, anim->x - area.x()), QMAX( 0, anim->y - area.y()));
            }
        }
    XCopyArea( qt_xdisplay(), pixmap, window, gc, 0, 0, area.width(), area.height(), area.x(), area.y());
    XFreePixmap( qt_xdisplay(), pixmap );
    }

static void createWindow()
    {
    assert( window == None );
#ifdef DEBUG
    fprintf( stderr, "GEOMETRY: %d %d %d %d\n", geometry.x(), geometry.y(), geometry.width(), geometry.height());
#endif
    XSetWindowAttributes attrs;
    attrs.override_redirect = True;
    attrs.background_pixmap = None;
//    attrs.override_redirect = False;
    window = XCreateWindow( qt_xdisplay(), DefaultRootWindow( qt_xdisplay()),
        geometry.x(), geometry.y(), geometry.width(), geometry.height(),
        0, CopyFromParent, CopyFromParent, CopyFromParent, CWOverrideRedirect | CWBackPixmap, &attrs );
    XSelectInput( qt_xdisplay(), window, ButtonPressMask | ExposureMask );
    XMapRaised( qt_xdisplay(), window );
    }

static void createSplashImage()
    {
    assert( splash_image.isNull());
    assert( splash_pixmap == None );
    splash_image = QImage( geometry.size(), 32 );
    splash_pixmap = XCreatePixmap( qt_xdisplay(), DefaultRootWindow( qt_xdisplay()),
        geometry.width(), geometry.height(), x11Depth());
    }

static bool waitState( int expected_state )
    {
    if( expected_state <= state )
        return false;
    if( window == None )
        createWindow();
    if( splash_image.isNull())
        {
        fprintf( stderr, "No window contents\n" );
        exit( 3 );
        }
    time_t test_time = time( NULL ) + 5;
#ifdef DEBUG
    fprintf( stderr,"AWATING STATE: %d\n", expected_state );
#endif
    if( parent_pipe >= 0 )
        { // wait for paint being finished, and tell parent to exit
        XSync( qt_xdisplay(), False );
        char buf = '\0';
        write( parent_pipe, &buf, 1 );
        close( parent_pipe );
        parent_pipe = -1;
        }
    for(;;)
        {
        while( XPending( qt_xdisplay()))
            {
            XEvent ev;
            XNextEvent( qt_xdisplay(), &ev );
            if( ev.type == ButtonPress && ev.xbutton.window == window && ev.xbutton.button == Button1 )
                {
                final_time = time( NULL );
                break;
                }
            if( ev.type == Expose && ev.xexpose.window == window )
                doPaint( QRect( ev.xexpose.x, ev.xexpose.y, ev.xexpose.width, ev.xexpose.height ));
            if( ev.type == ConfigureNotify && ev.xconfigure.event == DefaultRootWindow( qt_xdisplay()))
                XRaiseWindow( qt_xdisplay(), window );
            if( ev.type == ClientMessage && ev.xclient.window == DefaultRootWindow( qt_xdisplay())
                && ev.xclient.message_type == kde_splash_progress )
                {
                // based on ksplash
                const char* s = ev.xclient.data.b;
#ifdef DEBUG
                fprintf( stderr,"MESSAGE: %s\n", s );
#endif
                if( strcmp( s, "dcop" ) == 0 && state < 1 )
                    state = 1; // not actually used, state starts from 1, because dcop cannot be checked
                else if( strcmp( s, "kded" ) == 0 && state < 2 )
                    state = 2;
                else if( strcmp( s, "kcminit" ) == 0 )
                    ; // unused
                else if( strcmp( s, "ksmserver" ) == 0 && state < 3 )
                    state = 3;
                else if( strcmp( s, "wm started" ) == 0 && state < 4 )
                    state = 4;
                else if( strcmp( s, "kdesktop" ) == 0 && state < 5 )
                    state = 5;
                else if(( strcmp( s, "kicker" ) == 0 && state < 6 )
                    || ( strcmp( s, "session ready" ) == 0 && state < 7 ))
                    {
                    state = 7;
                    final_time = time( NULL ) + 1; // quit after short time
                    }
                }
            }
        if( test && time( NULL ) >= test_time )
            {
            ++state;
            test_time = time( NULL ) + 5;
            }
        if( expected_state <= state )
            return false;
        struct timeval tm_start, tm_end;
        gettimeofday( &tm_start, NULL );
        fd_set set;
        FD_ZERO( &set );
        FD_SET( XConnectionNumber( qt_xdisplay()), &set );
        int delay = 1000;
        for( int i = 0;
             i < MAX_ITEMS;
             ++i )
            if( animations[ i ] != NULL && animations[ i ]->delay < delay * 2 )
                delay = animations[ i ]->delay / 2;
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = delay * 1000;
        select( XConnectionNumber( qt_xdisplay()) + 1, &set, NULL, NULL, &tv );
        if( time( NULL ) >= final_time )
            {
#ifdef DEBUG
            fprintf( stderr, "EXITING\n" );
#endif
            return true; // --->
            }
        gettimeofday( &tm_end, NULL );
        int real_delay = (( tm_end.tv_sec - tm_start.tv_sec ) * 1000000 + tm_end.tv_usec - tm_start.tv_usec ) / 1000;
        for( int i = 0;
             i < MAX_ITEMS;
             ++i )
            {
            AnimData* anim = animations[ i ];
            if( anim != NULL && anim->updateFrame( real_delay ))
                doPaint( QRect( anim->x, anim->y, anim->frames[ anim->current_frame ].w, anim->frames[ anim->current_frame ].h ));
            }
        }
    }

static bool checkRelative( const char* ref )
    {
    if( ref[ 0 ] == '\0' || ref[ 1 ] == '\0' || ref[ 2 ] != '\0' )
        return false;
    if( strchr( "LRC", ref[ 0 ] ) == NULL )
        return false;
    if( strchr( "TBC", ref[ 1 ] ) == NULL )
        return false;
    return true;
    }

static int makeAbsolute( char screen, int val, char image, int size, int screen_size )
    {
    int pos;
    switch( screen )
        {
        case 'L':
        case 'T':
            pos = 0;
          break;
        case 'R':
        case 'B':
            pos = screen_size;
          break;
        case 'C':
            pos = screen_size / 2;
          break;
        default:
            exit( 3 );
        }
    pos += val;
    switch( image )
        {
        case 'L':
        case 'T':
            pos -= 0;
          break;
        case 'R':
        case 'B':
            pos -= size;
          break;
        case 'C':
            pos -= size / 2;
          break;
        default:
            exit( 3 );
        }
    return pos;
    }

static int makeAbsoluteX( const char* screen_ref, int x_rel, const char* image_ref, int width )
    {
    return makeAbsolute( screen_ref[ 0 ], x_rel, image_ref[ 0 ], width, geometry.width());
    }

static int makeAbsoluteY( const char* screen_ref, int y_rel, const char* image_ref, int height )
    {
    return makeAbsolute( screen_ref[ 1 ], y_rel, image_ref[ 1 ], height, geometry.height());
    }

static inline QRgb blend( QRgb c, QRgb background )
    {
    if( qAlpha( c ) == 255 )
        return c;
    return qRgb( ( qRed( background ) * ( 255 - qAlpha( c ) ) + qRed( c ) * qAlpha( c ) ) / 255,
                 ( qGreen( background ) * ( 255 - qAlpha( c ) ) + qGreen( c ) * qAlpha( c ) ) / 255,
                 ( qBlue( background ) * ( 255 - qAlpha( c ) ) + qBlue( c ) * qAlpha( c ) ) / 255 );
    }

static void blend( QImage& img, int x_pos, int y_pos, int x_img, int y_img, int w_img, int h_img )
    {
    if( !img.hasAlphaBuffer())
        return; // it doesn't have alpha, so it is the blended result
    for( int y = 0;
         y < h_img;
         ++y )
        {
        QRgb* s = (( QRgb* )( splash_image.scanLine( y + y_pos ))) + x_pos;
        QRgb* d = (( QRgb* )( img.scanLine( y + y_img ))) + x_img;
        for( int x = 0;
             x < w_img;
             ++x, ++d, ++s )
            {
            *d = blend( *d, *s );
            }
        }
    }

static void blend( QImage& img, int x_pos, int y_pos )
    {
    blend( img, x_pos, y_pos, 0, 0, img.width(), img.height());
    img.setAlphaBuffer( false );
    }

static void blendAnim( QImage& img, int x_pos, int y_pos, int frames )
    {
    int framew, frameh;
    frameSize( img, frames, framew, frameh );
    for( int frame = 0;
         frame < frames;
         ++frame )
        {
        blend( img, x_pos, y_pos,
            ( frame % ANIM_IMAGES_ROW ) * framew, ( frame / ANIM_IMAGES_ROW ) * frameh, framew, frameh );
        }
    img.setAlphaBuffer( false );
    }

static void updateSplashImage( const QImage& img, int x_pos, int y_pos )
    {
    for( int y = 0;
         y < img.height();
         ++y )
        {
        QRgb* s = (( QRgb* )( img.scanLine( y )));
        QRgb* d = (( QRgb* )( splash_image.scanLine( y + y_pos ))) + x_pos;
        for( int x = 0;
             x < img.width();
             ++x, ++d, ++s )
            {
            *d = *s;
            }
        }
    PixmapData pix = imageToPixmap( img );
    GC gc = qt_xget_temp_gc( x11Screen(), false );
    XCopyArea( qt_xdisplay(), pix.hd, splash_pixmap, gc, 0, 0, img.width(), img.height(), x_pos, y_pos );
    XFreePixmap( qt_xdisplay(), pix.hd );
    }

void runSplash( const char* them, bool t, int p )
    {
    geometry = screenGeometry();
    snprintf( theme_name, 1024, "%s", them );
    snprintf( theme_dir, 1024, "%s/ksplashx/Themes/%s", KDE_DATADIR, them );
    test = t;
    parent_pipe = p;
    anim_count = 0;
    state = 0;
    window = None;
    splash_image = QImage();
    splash_pixmap = None;
    final_time = time( NULL ) + 60;
    int desc_w, desc_h;
    FILE* datafile = fopen( findFile( "description.txt", &desc_w, &desc_h ), "r" );
    struct stat stat_buf;
    if( datafile == NULL || fstat( fileno( datafile ), &stat_buf ) != 0 )
        {
        fprintf( stderr, "Can't read description.txt file.\n" );
        exit( 2 );
        }
    timestamp = stat_buf.st_mtime;
    double ratiox = double( desc_w ) / screenGeometry().width(); // only for coordinates in the description file
    double ratioy = double( desc_h ) / screenGeometry().height(); // only for coordinates in the description file
    XSelectInput( qt_xdisplay(), DefaultRootWindow( qt_xdisplay()), SubstructureNotifyMask );
    kde_splash_progress = XInternAtom( qt_xdisplay(), "_KDE_SPLASH_PROGRESS", False );
    for( int i = 0;
         i < MAX_ITEMS;
         ++i )
        animations[ i ] = NULL;
    while( !feof( datafile ))
        {
        char line[ 1024 ];
        if( !freadline( line, 1024, datafile ))
            break;
        strip_whitespace( line );
        char buf[ 1024 ];
        int number, x, y, w, h, x_rel, y_rel, frames, delay;
        char screen_ref[ 3 ];
        char window_ref[ 3 ];
        char image_ref[ 3 ];
        if( line[ 0 ] == '#' || line[ 0 ] == '\0' )
            continue;
        else if( sscanf( line, "SCALE %1023s", buf ) == 1 )
            {
            if( strcmp( buf, "ON" ) == 0 )
                scale_on = true;
            else if( strcmp( buf, "OFF" ) == 0 )
                scale_on = false;
            else
                {
                fprintf( stderr, "Bad scale: %s\n", line );
                exit( 3 );
                }
            }
        else if( sscanf( line, "GEOMETRY %d %d %d %d", &x, &y, &w, &h ) == 4 )
            {
            if( scale_on )
                {
                x = round( x / ratiox );
                y = round( y / ratioy );
                w = round( w / ratiox );
                h = round( h / ratioy );
                }
            if( x < 0 )
                x += screenGeometry().width();
            if( y < 0 )
                y += screenGeometry().height();
            QRect r( x, y, w, h );
            if( screenGeometry().contains( r ))
                {
                geometry = r;
                if( window != None )
                    XMoveResizeWindow( qt_xdisplay(), window, x, y, w, h );
                if( !splash_image.isNull())
                    { // destroy and then recreate
                    splash_image = QImage();
                    XFreePixmap( qt_xdisplay(), splash_pixmap );
                    splash_pixmap = None;
                    }
                createSplashImage();
                }
            else
                {
                fprintf( stderr, "Wrong geometry: %s\n", line );
                exit( 3 );
                }
            }
        else if( sscanf( line, "GEOMETRY_REL %2s %d %d %2s %d %d",
            screen_ref, &x_rel, &y_rel, window_ref, &w, &h ) == 6 )
            {
            if( scale_on )
                {
                x_rel = round( x_rel / ratiox );
                y_rel = round( y_rel / ratioy );
                w = round( w / ratiox );
                h = round( h / ratioy );
                }
            if( !checkRelative( screen_ref )
                || !checkRelative( window_ref ))
                {
                fprintf( stderr,"Bad reference point: %s\n", line );
                exit( 3 );
                }
            x = makeAbsoluteX( screen_ref, x_rel, window_ref, w );
            y = makeAbsoluteY( screen_ref, y_rel, window_ref, h );
            QRect r( x, y, w, h );
            if( screenGeometry().contains( r ))
                {
                geometry = r;
                if( window != None )
                    XMoveResizeWindow( qt_xdisplay(), window, x, y, w, h );
                if( !splash_image.isNull())
                    { // destroy and then recreate
                    splash_image = QImage();
                    XFreePixmap( qt_xdisplay(), splash_pixmap );
                    splash_pixmap = None;
                    }
                createSplashImage();
                }
            else
                {
                fprintf( stderr, "Wrong geometry: %s\n", line );
                exit( 3 );
                }
            }
        else if( sscanf( line, "BACKGROUND %1023s", buf ) == 1 )
            {
            QColor background = QColor( buf );
            if( !background.isValid())
                {
                fprintf( stderr, "Bad color: %s\n", line );
                exit( 3 );
                }
            if( splash_image.isNull())
                createSplashImage();
            splash_image.fill( background.rgb());
            XGCValues xgc;
            xgc.foreground = background.pixel();
            GC gc = XCreateGC( qt_xdisplay(), splash_pixmap, GCForeground, &xgc );
            XFillRectangle( qt_xdisplay(), splash_pixmap, gc, 0, 0, geometry.width(), geometry.height());
            XFreeGC( qt_xdisplay(), gc );
            doPaint( QRect( 0, 0, geometry.width(), geometry.height()));
            }
        else if( sscanf( line, "IMAGE %d %d %1023s", &x, &y, buf ) == 3 )
            {
            if( scale_on )
                {
                x = round( x / ratiox );
                y = round( y / ratioy );
                }
            if( splash_image.isNull())
                createSplashImage();
            QImage img = loadImage( buf );
            if( !img.isNull())
                {
                if( !QRect( 0, 0, geometry.width(), geometry.height())
                    .contains( QRect( x, y, img.width(), img.height())))
                    {
                    fprintf( stderr, "Image outside of geometry: %s\n", line );
                    exit( 3 );
                    }
                blend( img, x, y );
                updateSplashImage( img, x, y );
                doPaint( QRect( x, y, img.width(), img.height()));
                }
            else
                {
                fprintf( stderr, "Bad image: %s\n", line );
                exit( 3 );
                }
            }
        else if( sscanf( line, "IMAGE_REL %2s %d %d %2s %1023s",
            window_ref, &x_rel, &y_rel, image_ref, buf ) == 5 )
            {
            if( scale_on )
                {
                x_rel = round( x_rel / ratiox );
                y_rel = round( y_rel / ratioy );
                }
            if( !checkRelative( window_ref )
                || !checkRelative( window_ref ))
                {
                fprintf( stderr,"Bad reference point: %s\n", line );
                exit( 3 );
                }
            if( splash_image.isNull())
                createSplashImage();
            QImage img = loadImage( buf );
            if( !img.isNull())
                {
                x = makeAbsoluteX( window_ref, x_rel, image_ref, img.width());
                y = makeAbsoluteY( window_ref, y_rel, image_ref, img.height());
                if( !QRect( 0, 0, geometry.width(), geometry.height())
                    .contains( QRect( x, y, img.width(), img.height())))
                    {
                    fprintf( stderr, "Image outside of geometry: %s\n", line );
                    exit( 3 );
                    }
                blend( img, x, y );
                updateSplashImage( img, x, y );
                doPaint( QRect( x, y, img.width(), img.height()));
                }
            else
                {
                fprintf( stderr, "Bad image: %s\n", line );
                exit( 3 );
                }
            }
        else if( sscanf( line, "ANIM %d %d %d %d %1023s %d", &number, &x, &y, &frames, buf, &delay ) == 6 )
            {
            if( scale_on )
                {
                x = round( x / ratiox );
                y = round( y / ratioy );
                }
            if( number <= 0 || number >= MAX_ITEMS )
                {
                fprintf( stderr,"Bad number: %s\n", line );
                exit( 3 );
                }
            if( frames <= 0 || frames > MAX_ITEMS )
                {
                fprintf( stderr, "Frames limit reached: %s\n", line );
                exit( 3 );
                }
            if( splash_image.isNull())
                createSplashImage();
            QImage imgs = loadAnimImage( buf, frames );
            if( !imgs.isNull())
                {
                blendAnim( imgs, x, y, frames );
                PixmapData* pixs = imageAnimToPixmaps( imgs, frames );
                delete animations[ number ];
                animations[ number ] = new AnimData( x, y, pixs, frames, delay );
                }
            }
        else if( sscanf( line, "ANIM_REL %d %2s %d %d %2s %d %1023s %d",
            &number, window_ref, &x_rel, &y_rel, image_ref, &frames, buf, &delay ) == 8 )
            {
            if( scale_on )
                {
                x_rel = round( x_rel / ratiox );
                y_rel = round( y_rel / ratioy );
                }
            if( number <= 0 || number >= MAX_ITEMS )
                {
                fprintf( stderr,"Bad number: %s\n", line );
                exit( 3 );
                }
            if( !checkRelative( window_ref )
                || !checkRelative( window_ref ))
                {
                fprintf( stderr,"Bad reference point: %s\n", line );
                exit( 3 );
                }
            if( frames <= 0 || frames > MAX_ITEMS )
                {
                fprintf( stderr, "Frames limit reached: %s\n", line );
                exit( 3 );
                }
            if( splash_image.isNull())
                createSplashImage();
            QImage imgs = loadAnimImage( buf, frames );
            if( !imgs.isNull())
                {
                int framew, frameh;
                frameSize( imgs, frames, framew, frameh );
                x = makeAbsoluteX( window_ref, x_rel, image_ref, framew );
                y = makeAbsoluteY( window_ref, y_rel, image_ref, frameh );
                blendAnim( imgs, x, y, frames );
                PixmapData* pixs = imageAnimToPixmaps( imgs, frames );
                delete animations[ number ];
                animations[ number ] = new AnimData( x, y, pixs, frames, delay );
                }
            }
        else if( sscanf( line, "STOP_ANIM %d", &number ) == 1 )
            {
            if( number <= 0 || number >= MAX_ITEMS || animations[ number ] == NULL )
                {
                fprintf( stderr,"Bad number: %s\n", line );
                exit( 3 );
                }
            AnimData* anim = animations[ number ];
            doPaint( QRect( anim->x, anim->y, anim->frames[ 0 ].w, anim->frames[ 0 ].h ));
            delete animations[ number ];
            animations[ number ] = NULL;
            }
        else if( sscanf( line, "WAIT_STATE %d", &number ) == 1 )
            {
            if( number < 0 || number > LAST_STATE + 1 )
                {
                fprintf( stderr, "Bad state: %i\n", number );
                exit( 3 );
                }
            if( waitState( number ))
                break; // exiting
            }
        else
            {
            fprintf( stderr, "Unknown line: %s\n", line );
            exit( 3 );
            }
        }
    fclose( datafile );
    XDestroyWindow( qt_xdisplay(), window );
    window = None;
    XFreePixmap( qt_xdisplay(), splash_pixmap );
    splash_pixmap = None;
    splash_image = QImage();
    }
