/*
 * Copyright (c) 2011-2014 BlackBerry Limited.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import bb.cascades 1.4
import bb.device 1.4
import bb.system 1.2

Page {
    property variant vpoint: 0
    attachedObjects: [
        
        DisplayInfo {
            id: display
        
        }
    ]
    Container {
        horizontalAlignment: HorizontalAlignment.Fill
        attachedObjects: [
            LayoutUpdateHandler {
                id: handler
            }
        ]
        layout: StackLayout {
        
        }
        SegmentedControl {
            id: segcontrol
            onSelectedIndexChanged: {
                console.log("selected index", selectedIndex)
                if (selectedIndex == 0) {
                    scrollview.scrollToPoint(first.layoutFrame.x, first.layoutFrame.y)
                } else if(selectedIndex == 1)
                    scrollview.scrollToPoint(second.layoutFrame.x, second.layoutFrame.y)
                else if(selectedIndex == 2)
                    scrollview.scrollToPoint(third.layoutFrame.x, third.layoutFrame.y)
                else if(selectedIndex == 3)
                    scrollview.scrollToPoint(fourth.layoutFrame.x, fourth.layoutFrame.y)
            
            
            }
            options: [
                Option {
                    id: one
                    text: "one"
                    value: "1"
                    selected: true
                
                },
                Option {
                    id: two
                    text: "two"
                    value: "2"
                
                },
                Option {
                    id: three
                    text: "three"
                    value: "3"
                
                },
                Option {
                    id: four
                    text: "four"
                    value: "4"
                
                }
            ]
        }
        Container {
            verticalAlignment: VerticalAlignment.Fill
            ScrollView {
                id: scrollview
                onTouch: {
                    if (event.isUp()) {
                        console.log("this is the x,y", vpoint,second.layoutFrame.x, second.layoutFrame.y)
                        if (0 < vpoint  && vpoint < display.pixelSize.width/2) {
                            segcontrol.setSelectedIndex(0)
                            scrollview.scrollToPoint(first.layoutFrame.x, first.layoutFrame.y)
                        
                        } else if(display.pixelSize.width/2 < vpoint && vpoint < display.pixelSize.width*3/2){
                            segcontrol.setSelectedIndex(1)
                            
                            scrollview.scrollToPoint(second.layoutFrame.x, second.layoutFrame.y)
                        
                        }
                        else if(display.pixelSize.width*3/2 < vpoint && vpoint < display.pixelSize.width*5/2){
                            segcontrol.setSelectedIndex(2)
                            scrollview.scrollToPoint(third.layoutFrame.x, third.layoutFrame.y)
                        
                        }
                        else if(display.pixelSize.width*5/2 < vpoint && vpoint < display.pixelSize.width*7/2){
                            segcontrol.setSelectedIndex(3)
                            scrollview.scrollToPoint(fourth.layoutFrame.x, fourth.layoutFrame.y)
                        
                        }
                    
                    }
                
                }
                onViewableAreaChanging: {
                    vpoint = viewableArea.x
                    console.log(vpoint)
                }
                
                scrollRole: ScrollRole.Main
                scrollViewProperties.scrollMode: ScrollMode.Horizontal
                horizontalAlignment: HorizontalAlignment.Fill
                Container {
                    horizontalAlignment: HorizontalAlignment.Fill
                    minHeight: display.physicalSize.height
                    layout: StackLayout {
                        orientation: LayoutOrientation.LeftToRight
                    }
                    Container {
                        id: oneContent
                        minWidth: handler.layoutFrame.width
                        verticalAlignment: VerticalAlignment.Fill
                        background: Color.Blue
                        attachedObjects: [
                            LayoutUpdateHandler {
                                id: first
                            }
                        ]
                        Label {
                            text: "this is 1"
                        }
                    }
                    Container {
                        minHeight: display.pixelSize.height
                        attachedObjects: [
                            LayoutUpdateHandler {
                                id: second
                            }
                        ]
                        id: twoContent
                        minWidth: handler.layoutFrame.width
                        verticalAlignment: VerticalAlignment.Fill
                        background: Color.Red
                        Label {
                            text: "this is 2"
                        }
                    }
                    Container {
                        id: threeContent
                        minWidth: handler.layoutFrame.width
                        verticalAlignment: VerticalAlignment.Fill
                        background: Color.Blue
                        attachedObjects: [
                            LayoutUpdateHandler {
                                id: third
                            }
                        ]
                        Label {
                            text: "this is 1"
                        }
                    }
                    Container {
                        minHeight: display.pixelSize.height
                        attachedObjects: [
                            LayoutUpdateHandler {
                                id: fourth
                            }
                        ]
                        id: fourcontent
                        minWidth: handler.layoutFrame.width
                        verticalAlignment: VerticalAlignment.Fill
                        background: Color.Cyan
                        Label {
                            text: "this is 2"
                        }
                    }
                
                }
            }
        }
    }
}
