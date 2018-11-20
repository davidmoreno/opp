import * as React from "react";

export interface TopProps {};

export function TopMenu(_props: TopProps){
  return (
    <div className="ui horizontal menu" id="topmenu">
      <span className="stretch"/>
      <a className="item">Login</a>
    </div>
  )
}
