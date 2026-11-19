from pathlib import Path
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
flags = ["gcc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-fno-pie", "-no-pie",
         "-Wno-int-to-pointer-cast", "-Wno-pointer-to-int-cast",
         "-Isrc", "-Ihardware", "-Ihardware-edited"]
with tempfile.TemporaryDirectory(prefix="sensor-tests-") as folder:
    folder = Path(folder)
    def run(args):
        subprocess.run(args, cwd=root, check=True)
    run(flags + ["tests/check_logic.c", "src/motor_controller.c", "-o", str(folder / "logic")])
    run([str(folder / "logic")])
    # Replace the physical wait; the ADC test checks the requested startup delay.
    run(flags + ["-DdelayMicroseconds=hardwareDelayMicroseconds", "-c",
                 "hardware-edited/stm32f446xx_timer_mort.c", "-o", str(folder / "timer.o")])
    run(flags + ["tests/check_registers.c", "src/sensors.c", "src/motor.c", "src/motor_controller.c",
                 "hardware-edited/stm32f446xx_gpio_mort.c",
                 "hardware-edited/stm32f446xx_adc_mort.c",
                 "hardware-edited/stm32f446xx_dma_controller_mort.c",
                 str(folder / "timer.o"), "-o", str(folder / "registers")])
    run([str(folder / "registers")])
