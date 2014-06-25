//                                      __
//     ____ ___  ___  ____ ___  ____   / /__   __
//    / __ `__ \/ _ \/ __ `__ \/ __ \ / __/ | / /
//   / / / / / /  __/ / / / / / /_/ // /_ | |/ /
//  /_/ /_/ /_/\___/_/ /_/ /_/\____(_)__/ |___/
//
//
//  Created by Memo Akten, www.memo.tv
//
//  ofxMSAControlFreakGui
//


#include "ofxMSAControlFreakGui/src/ofxMSAControlFreakGui.h"

namespace msa {
    namespace controlfreak {
        namespace gui {
            
            
//            Slider2d::Slider2d(Container *parent, Parameter *p) : Control(parent, p) {
//                min.set(xmin, ymin);
//                max.set(xmax, ymax);
//                this->value = &value;
//                setup();
//            }
//            
//            void Slider2d::setup() {
//                setSize(getConfig()->layout.slider2DSize.x, getConfig()->layout.slider2DSize.y + getConfig()->layout.slider2DTextHeight);
//                point.x = ofMap((*value).x, min.x, max.x, x, x+width);
//                point.y = ofMap((*value).y, min.y, max.y, y, y+height-getConfig()->layout.slider2DTextHeight);
//            }
////            
////            void Slider2d::readFromXml(ofxXmlSettings &XML) {
////                value->set(XML.get(controlType + "_" + key + ":valueX", 0.0f), XML.get(controlType + "_" + key + ":valueY", 0.0f));
////            }
////            
////            
////            void Slider2d::writeToXml(ofxXmlSettings &XML) {
////                XML.addTag(controlType + "_" + key);
////                XML.pushTag(controlType + "_" + key);
////                XML.addValue("name", name);
////                XML.addValue("valueX", value->x);
////                XML.addValue("valueY", value->y);
////                XML.popTag();
////            }
//            
//            void Slider2d::set(float x, float y) {
//                (*value).x = x;
//                (*value).y = y;
//            }
//            
//            void Slider2d::setMin(float x, float y) {
//                min.x = x;
//                min.y = y;
//            }
//            
//            void Slider2d::setMax(float x, float y) {
//                max.x = x;
//                max.y = y;
//            }
//            
//            void Slider2d::onPress(int x, int y, int button) {
//                lock = true;
//                point.set(x, y);
//            }
//            
//            void Slider2d::onDragOver(int x, int y, int button) {
//                if(lock) {
//                    point.set(x, y);
//                }
//            }
//            
//            void Slider2d::onDragOutside(int x, int y, int button) {
//                if(lock) {
//                    point.set(x, y);
//                }
//            }
//            
//            void Slider2d::onRelease() {
//                lock = false;
//            }
//            
//            void Slider2d::update() {
//                if(point.x > x + width)				point.x = x + width;
//                else if(point.x < x)				point.x = x;
//                
//                if(point.y > y+height - getConfig()->layout.slider2DTextHeight)			point.y = y + height - getConfig()->layout.slider2DTextHeight;
//                else if(point.y < y)				point.y = y;
//                
//                if(lock){
//                    (*value).x = ofMap(point.x, x, x+width, min.x, max.x);
//                    (*value).y = ofMap(point.y, y, y+height-getConfig()->layout.slider2DTextHeight, min.y, max.y);
//                }
//            }
//            
//            void Slider2d::draw(float x, float y) {
//                setPos(x, y);
//                ofPoint	pointv;
//                pointv.x = ofMap((*value).x, min.x, max.x, x, x+width);
//                pointv.y = ofMap((*value).y, min.y, max.y, y, y+height-getConfig()->layout.slider2DTextHeight);
//                
//                ofEnableAlphaBlending();
//                glPushMatrix();
//                glTranslatef(x, y, 0);
//                
//                ofFill();
//                setColor(getConfig()->colors.slider.full);
//                ofRect(0, 0, width, height - getConfig()->layout.slider2DTextHeight);
//                
//                ofFill();
//                setBGColor();
//                ofRect(0, height-getConfig()->layout.slider2DTextHeight, width, getConfig()->layout.slider2DTextHeight);
//                
//                setTextColor();
//                getConfig()->drawString(name+"\nx:"+ofToString(value->x, 2)+"\ny:"+ofToString(value->y, 2), 3, height+15-getConfig()->layout.slider2DTextHeight);
//                
//                setTextColor();
//                ofCircle(pointv.x-x, pointv.y-y, 2);
//                
//                setTextColor();
//                ofLine(pointv.x-x, 0, pointv.x-x, height-getConfig()->layout.slider2DTextHeight);
//                ofLine(0, pointv.y-y,width, pointv.y-y);
//                
//                glPopMatrix();
//                ofDisableAlphaBlending();
//                
//            }
        }
    }
}