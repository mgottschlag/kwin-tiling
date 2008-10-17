

function createGenericButton(placeElement,buttonText,clickHandler) {
  var b = document.createElement("button");
  b.appendChild(document.createTextNode(buttonText));
  b.onclick = clickHandler;
  placeElement.appendChild(b);
}