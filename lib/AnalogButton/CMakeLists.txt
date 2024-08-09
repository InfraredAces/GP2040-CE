add_library(AnalogButton sma_filter.cpp)
target_link_libraries(AnalogButton PUBLIC PicoPeripherals)
target_include_directories(AnalogButton INTERFACE .)
target_include_directories(AnalogButton PUBLIC . PicoPeripherals)
