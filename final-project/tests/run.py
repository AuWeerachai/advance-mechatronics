from pathlib import Path
import subprocess
import tempfile
import re

root = Path(__file__).resolve().parents[1]
flags = ["gcc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-fno-pie", "-no-pie",
         "-Wno-int-to-pointer-cast", "-Wno-pointer-to-int-cast",
         "-Isrc", "-Ihardware", "-Ihardware-edited"]
with tempfile.TemporaryDirectory(prefix="tower-tests-") as folder:
    folder = Path(folder)
    def run(args):
        subprocess.run(args, cwd=root, check=True)
    # Test the same firmware with both requested cycle settings, without editing the project.
    main_source = (root / "src/main.c").read_text()
    for cycles in (2, 5):
        source, replacements = re.subn(
            r"(#define PUMPS_PER_AIR_LED\s+\(\(uint32_t\))\d+(\))",
            lambda match: match[1] + str(cycles) + match[2], main_source)
        assert replacements == 1
        variant = folder / f"main_{cycles}.c"
        variant.write_text(source)
        executable = folder / f"logic_{cycles}"
        run(flags + [f'-DTEST_MAIN_SOURCE="{variant}"', "tests/check_logic.c", "-o", str(executable)])
        run([str(executable)])
    # Replace the timer's physical wait only in this host test; ADC setup uses the stub.
    run(flags + ["-DdelayMicroseconds=hardwareDelayMicroseconds", "-c",
                 "hardware-edited/stm32f446xx_timer_mort.c", "-o", str(folder / "timer.o")])
    run(flags + ["tests/check_registers.c", "src/sensors.c", "src/led.c", "hardware-edited/stm32f446xx_gpio_mort.c",
                 "hardware-edited/stm32f446xx_adc_mort.c",
                 "hardware-edited/stm32f446xx_dma_controller_mort.c",
                 str(folder / "timer.o"), "-o", str(folder / "registers")])
    run([str(folder / "registers")])
