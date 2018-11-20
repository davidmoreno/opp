import * as React from "react";
import * as ReactDOM from "react-dom";

import { Hello } from "./components/Hello";
declare let module: any


ReactDOM.render(
  <Hello compiler="TypeScript" framework="React"/>,
  document.getElementById("example")
)

if (module.hot) {
   module.hot.accept();
}
