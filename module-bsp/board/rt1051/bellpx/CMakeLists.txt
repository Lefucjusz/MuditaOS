target_include_directories(
        module-bsp
    PUBLIC
        $<BUILD_INTERFACE:
            ${CMAKE_CURRENT_SOURCE_DIR}
        >
)

target_sources(
        module-bsp

    PRIVATE
        hal/battery_charger/BatteryCharger.cpp
        hal/key_input/KeyInput.cpp
        hal/temperature_source/TemperatureSource.cpp

        bsp/audio/AW8898driver.cpp
        bsp/audio/CodecAW8898.cpp

        bsp/eink/eink_gpio.cpp
        bsp/lpm/PowerProfile.cpp
        bsp/lpm/RT1051LPM.cpp
        bsp/lpm/WfiController.cpp
        bsp/lpm/EnterSleepMode.c
        bsp/rotary_encoder/rotary_encoder.cpp
        bsp/rtc/rtc_configuration.cpp
        bsp/switches/switches.cpp

        audio.cpp
        clock_config.cpp
        debug_console.cpp
        irq_gpio.cpp
        pin_mux.c
        board.cpp
        brownout.cpp

    PUBLIC
        eink-config.h
        board/pin_mux.h
        board/clock_config.h
        board/irq_gpio.hpp
        board/brownout.hpp
)
