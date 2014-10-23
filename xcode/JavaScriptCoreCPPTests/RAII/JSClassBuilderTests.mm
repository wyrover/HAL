//
//  JSClassDefinitionBuilderTests.m
//  JavaScriptCoreCPP
//
//  Created by Matt Langston on 10/22/14.
//  Copyright (c) 2014 Appcelerator. All rights reserved.
//

#include "JavaScriptCoreCPP/RAII/RAII.hpp"
#include "DerivedJSObject.hpp"
#import <XCTest/XCTest.h>

using namespace JavaScriptCoreCPP::RAII;

@interface JSClassDefinitionBuilderTests : XCTestCase
@end

@implementation JSClassDefinitionBuilderTests {
  JSContext js_context;
}


- (void)setUp {
    [super setUp];
    // Put setup code here. This method is called before the invocation of each test method in the class.
}

- (void)tearDown {
    // Put teardown code here. This method is called after the invocation of each test method in the class.
    [super tearDown];
}

- (void)testJSClassBuilder {
  JSClassBuilder builder("Foo");
  //builder.set_initialize_callback(&DerivedJSObject::Initialize);
  auto js_class = builder.build();
  JSObject js_object(js_class, js_context);
  
  using InitializeCallback = std::function<void(DerivedJSObject&)>;
  InitializeCallback foo = &DerivedJSObject::Initialize;
}

//- (void)testJSClassBuilder2 {
//  JSClassBuilder2<DerivedJSObject> builder2("Foo");
//  builder2.set_initialize_callback(&DerivedJSObject::Initialize);
//  auto js_class = builder2.build();
//  JSObject js_object(js_class, js_context);
//}

// As of 2014.09.20 Travis CI only supports Xcode 5.1 which lacks support for
// measureBlock.
#ifndef TRAVIS
- (void)testPerformanceExample {
  // This is an example of a performance test case.
  [self measureBlock:^{
    // Put the code you want to measure the time of here.
  }];
}
#endif

@end