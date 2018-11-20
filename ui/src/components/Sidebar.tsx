import * as React from "react";

export interface SidebarProps {};

const logo = require("../logo.png");

export function Sidebar(_props: SidebarProps){
  return (
    <div className="ui vertical menu" id="sidebar">
      <a className="ui padding" href="https://www.coralbits.com/" target="_blank">
        <img src={logo} style={{maxHeight: "60px"}}/>
      </a>
      <a className="item">Top</a>
      <a className="item">Processes</a>
      <span className="stretch"/>
      <hr/>
      <a className="item" href="https://github.com/davidmoreno/opp/" target="_blank">GitHub</a>
    </div>
  )
}
