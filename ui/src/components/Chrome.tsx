import * as React from "react";
import { TopMenu } from "./TopMenu";
import { Sidebar } from "./Sidebar";
import { MainArea } from "./MainArea";

require("../sass/chrome.sass")

export interface ChromeProps {}

export function Chrome(props: ChromeProps){
  return (
    <React.Fragment>
      <TopMenu {...props}/>
      <Sidebar {...props}/>
      <MainArea {...props}/>
    </React.Fragment>
  )
}
