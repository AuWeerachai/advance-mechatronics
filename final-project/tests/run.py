from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
flags = ["gcc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-fno-pie", "-no-pie",
         "-Wno-int-to-pointer-cast", "-Wno-pointer-to-int-cast",
         "-Isrc", "-Ihardware", "-Ihardware-edited"]
with tempfile.TemporaryDirectory(prefix="game-tests-") as folder:
    folder = Path(folder)
    def run(args):
        subprocess.run(args, cwd=root, check=True)
    run(flags + ["tests/check_framework.c", "src/framework.c", "-o", str(folder / "framework")])
    run([str(folder / "framework")])
    run(flags + ["tests/check_serial.c", "src/serial.c", "-o", str(folder / "serial")])
    run([str(folder / "serial")])
    game = ["tests/check_logic.c", "src/motor_controller.c", "src/events.c",
            "src/framework.c", "src/services.c", "src/state_machine.c", "src/serial.c", "src/targets.c"]
    # Test-only geometry. These values are not written into production firmware.
    geometry = ["-DTARGET_LED_1_DISTANCE_CM=30", "-DTARGET_LED_2_DISTANCE_CM=45", "-DTARGET_LED_3_DISTANCE_CM=60"]
    run(flags + game + ["-o", str(folder / "unconfigured")])
    run([str(folder / "unconfigured")])
    for name, config in [("default", []), ("configurable", ["-DTARGET_ROUND_COUNT=2",
                         "-DTARGET_REPOSITION_US=30000000", "-DTARGET_HOLD_US=1000000", "-DPUMPS_PER_AIR_LED=5", "-DSTART_BUTTON_DEBOUNCE_US=50000"])]:
        run(flags + geometry + config + game + ["-o", str(folder / name)])
        run([str(folder / name)])
        run(flags + geometry + config + ["tests/check_targets.c", "src/targets.c", "-o", str(folder / "targets")])
        run([str(folder / "targets")])
    # Feed raw ADC traces through the real calibration, events and state machines.
    run(flags + geometry + ["tests/check_sensor_noise.c", "src/sensors.c", "src/events.c",
                 "src/framework.c", "src/services.c", "src/state_machine.c", "src/serial.c",
                 "src/targets.c", "src/motor_controller.c", "-o", str(folder / "noise")])
    run([str(folder / "noise")])
    for connected in [0, 1]:
        run(flags + [f"-DFAN_TACH_CONNECTED={connected}", "tests/check_tachometer.c", "src/tachometer.c",
                     "hardware-edited/stm32f446xx_interruptcontroller_mort.c",
                     "-o", str(folder / "tach")])
        run([str(folder / "tach")])
    # Replace the physical wait; the ADC test checks the requested startup delay.
    run(flags + ["-DdelayMicroseconds=hardwareDelayMicroseconds", "-c",
                 "hardware-edited/stm32f446xx_timer_mort.c", "-o", str(folder / "timer.o")])
    run(flags + ["tests/check_registers.c", "src/sensors.c", "src/motor.c", "src/motor_controller.c",
                 "hardware-edited/stm32f446xx_gpio_mort.c",
                 "hardware-edited/stm32f446xx_adc_mort.c",
                 "hardware-edited/stm32f446xx_dma_controller_mort.c",
                 str(folder / "timer.o"), "-o", str(folder / "registers")])
    run([str(folder / "registers")])
