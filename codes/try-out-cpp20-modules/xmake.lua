set_languages("c++20")
target("hello")
  add_cxflags("-fmodules-ts")
  add_files("*.cpp")
