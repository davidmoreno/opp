import * as React from "react";

export interface SidebarProps {};

export function Sidebar(_props: SidebarProps){
  return (
    <div className="ui vertical menu" id="sidebar">
      <a className="item">Top</a>
      <a className="item">Processes</a>
    </div>
  )
}
