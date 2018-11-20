import * as React from "react";
import * as ReactDOM from "react-dom";

import { Chrome } from "./components/Chrome";
declare let module: any


ReactDOM.render(
  <Chrome />,
  document.getElementById("chrome")
)

if (module.hot) {
   module.hot.accept();
}
