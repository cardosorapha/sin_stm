# SIN_STM
Gerador de senoide utilizando microcontrolador STM32F103C8T6 e conversor D/A DAC0800LCN. Construído com base no toolchain disponível [neste](https://github.com/jeffsjunior/STM32_Toolchain) repositório.

## Utilização
Na pasta **Projects**, execute 

```
git clone https://github.com/cardosorapha/sin_stm
```

Após isso, basta modificar o código fonte em **src**. Para compilar e gravar, basta utilizar

```
make erflash
```
__Atenção:__ esse programa utiliza a biblioteca de C <math.h>. Portanto, no arquivo **Makefile.mk**, a flag *-lm* deve estar presente nos *LDLIBS*, por exemplo, a linha 85 do **Makefile.mk** pode ser 

```
LDLIBS		+= -Wl,--start-group -lc -lm -lgcc -lnosys -Wl,--end-group
```
