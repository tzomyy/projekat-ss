ASSEMBLER=./assembler
LINKER=./linker
EMULATOR=./emulator
PATH=./tests/

${ASSEMBLER} -o main.o ./tests/main.s
# ${ASSEMBLER} -o math.o ./tests/math.s
# ${ASSEMBLER} -o ivt.o ./tests/ivt.s
# ${ASSEMBLER} -o isr_reset.o ./tests/isr_reset.s
# ${ASSEMBLER} -o isr_terminal.o ./tests/isr_terminal.s
# ${ASSEMBLER} -o isr_timer.o ./tests/isr_timer.s
# ${ASSEMBLER} -o isr_user0.o ./tests/isr_user0.s
# ${LINKER} -hex -g -o program.hex ivt.o math.o main.o isr_reset.o isr_terminal.o isr_timer.o isr_user0.o
# ${EMULATOR} program.hex

# ${ASSEMBLER} -o test.o ./tests/test.s

# ${ASSEMBLER} -o main.o ${PATH}main.s
# ${ASSEMBLER} -o ivt.o ${PATH}ivt.s
# ${ASSEMBLER} -o isr_reset.o ${PATH}isr_reset.s
# ${ASSEMBLER} -o isr_terminal.o ${PATH}isr_terminal.s
# ${ASSEMBLER} -o isr_timer.o ${PATH}isr_timer.s
# ${LINKER} -hex -place=ivt@0x0000 -o program.hex main.o isr_reset.o isr_terminal.o isr_timer.o ivt.o