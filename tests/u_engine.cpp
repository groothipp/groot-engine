#include "src/include/engine.hpp"

#include <catch2/catch_test_macros.hpp>
#include <iostream>

TEST_CASE( "engine", "[unit][engine]" ) {
  ge::Engine engine;

  SECTION( "add_duplicate_material" ) {
    engine.add_material("test", ge::MaterialManager::Builder());

    bool caught = false;
    try {
      engine.add_material("test", ge::MaterialManager::Builder());
    }
    catch (const std::runtime_error& e) {
      caught = true;
      CHECK( e.what() == std::string("groot-engine: material 'test' already exists") );
    }
    CHECK( caught );
  }

  SECTION( "object_with_invalid_material" ) {
    bool caught = false;
    try {
      engine.add_object("test", "../tests/dat/quad.obj");
    }
    catch (const std::runtime_error& e) {
      caught = true;
      CHECK( e.what() == std::string("groot-engine: material 'test' does not exist"));
    }
    CHECK( caught );
  }

  SECTION( "full_test" ) {
    engine.add_material("test", ge::MaterialManager::Builder()
      .add_shader(ge::ShaderStage::VertexShader, "shaders/shader.vert.spv")
      .add_shader(ge::ShaderStage::FragmentShader, "shaders/shader.frag.spv")
    );

    engine.add_material("test2", ge::MaterialManager::Builder()
      .add_shader(ge::ShaderStage::VertexShader, "shaders/shader.vert.spv")
      .add_shader(ge::ShaderStage::FragmentShader, "shaders/shader2.frag.spv")
    );

    engine.add_material("test3", ge::MaterialManager::Builder()
      .add_shader(ge::ShaderStage::VertexShader, "shaders/shader.vert.spv")
      .add_shader(ge::ShaderStage::FragmentShader, "shaders/shader3.frag.spv")
    );

    ge::transform obj1 = engine.add_object("test", "../tests/dat/circle.obj",
      ge::Transform(ge::vec3(-1.5f, -0.75f, 0.0f), ge::vec3(0.0f), ge::vec3(0.3))
    );

    ge::transform obj2 = engine.add_object("test", "../tests/dat/circle.obj",
      ge::Transform(ge::vec3(-1.5f, -0.75f, 0.15f), ge::vec3(0.0f), ge::vec3(0.3f))
    );

    ge::transform obj3 = engine.add_object("test2", "../tests/dat/quad.obj",
      ge::Transform(ge::vec3(1.5f, 0.75f, 0.0f), ge::vec3(0.0f), ge::vec3(0.6f))
    );

    ge::transform obj4 = engine.add_object("test2", "../tests/dat/quad.obj",
      ge::Transform(ge::vec3(1.5f, 0.75f, 0.15f), ge::vec3(0.0f), ge::vec3(0.6f))
    );

    ge::transform obj5 = engine.add_object("test3", "../tests/dat/pentagon.obj",
      ge::Transform(ge::vec3(-1.5f, 0.0f, -0.15f), ge::vec3(0.0f), ge::vec3(0.23f))
    );

    ge::transform obj6 = engine.add_object("test3", "../tests/dat/pentagon.obj",
      ge::Transform(ge::vec3(0.0f, -0.75f, -0.15f), ge::vec3(0.0f), ge::vec3(0.23f))
    );

    std::tuple<
      ge::transform&, ge::transform&, ge::transform&, ge::transform&, ge::transform&, ge::transform&
    > transforms = { obj1, obj2, obj3, obj4, obj5, obj6 };

    float w = ge::radians(5.0f), ax = 1.5f, ay = 0.75f;

    bool success = true;
    try {
      engine.run([&transforms, &w, &ax, &ay](double dt) {
        auto& [obj1, obj2, obj3, obj4, obj5, obj6] = transforms;

        float dx = w * ax * std::sin(w * obj1->elapsed_time()) * dt;
        float dy = w * ay * std::sin(w * obj1->elapsed_time()) * dt;

        obj1->translate({ dx, 0.0f, 0.0f });
        obj2->translate({ 0.0f, dy, 0.0f });
        obj3->translate({ -dx, 0.0f, 0.0f });
        obj4->translate({ 0.0f, -dy, 0.0f });
        obj5->translate({ dx, 0.0f, 0.0f });
        obj6->translate({ 0.0f, dy, 0.0f });
      });
    }
    catch (const std::exception& e) {
      success = false;
      std::cout << e.what() << '\n';
    }
    CHECK( success );
  }
}