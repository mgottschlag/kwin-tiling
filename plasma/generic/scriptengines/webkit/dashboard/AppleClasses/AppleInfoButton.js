 
function AppleInfoButton( placeElement, unknown, colorA, colorB, clickHandler){

var b = document.createElement("button");
  b.appendChild(document.createTextNode("i"));
  b.onclick = clickHandler;
  placeElement.appendChild(b);

}