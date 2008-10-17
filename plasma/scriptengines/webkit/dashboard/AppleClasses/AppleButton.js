 
function AppleButton( placeElement,buttonText, height, 
          buttonLeftImg,buttonLeftDownImg, leftWidth,
          buttonMiddleImg,buttonMiddleDownImg, buttonRightImg, buttonRightDown, rightWidth,
          clickHandler){
  var b = document.createElement("button");
  b.appendChild(document.createTextNode(buttonText));
  b.onclick = clickHandler;
  placeElement.appendChild(b);
}

function AppleGlassButton( placeElement,buttonText, clickHandler){
  var b = document.createElement("button");
  b.appendChild(document.createTextNode(buttonText));
  b.onclick = clickHandler;
  placeElement.appendChild(b);
}