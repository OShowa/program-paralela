## Questão 2.

### Processo de desenvolvimento:

A compilação do arquivo sequencial disponibilizado no AVA ocorreu sem intercorrências. O primeiro problema que o grupo encontrou foi na hora de executar o programa: a imagem gerada e apresentada na janela criada no X11 tinha a sua parte de cima invisível. Foi postulado que isso ocorria por conta do tempo que a janela demorava pra abrir (nesse intervalo, o loop já estaria rodando), então o programa tentava desenhar a metade de cima da imagem ainda antes de ter uma janela aberta disponível.

Para corrigir, foi simplesmente incluído um sleep de 1 segundo entre o pedido de abertura da janela para o server X11 e o início do loop principal, tempo mais que suficiente para a janela abrir.

O mandelbrot colorido foi relativamente fácil de implementar: por meio da função `XSetForeground`, definimos a cor do pixel a ser desenhado, que é calculada com base em k. A escolha de k em particular é por efeitos visuais: se o número diverge rapidamente, ele tende a ficar mais escuro, além de podermos escolher o tom exato que queremos para o conjunto em si definindo k = 100.

![image](../images/fluzao_mandelbrot.png)

A paralelização do código foi feita por meio de duas diretivas do OpenMP. A primeira diz respeito ao loop principal:

```c++
#pragma omp parallel for collapse(2) shared(display, win, gc) private(z, c, k, temp, lengthsq)
```

Foram declaradas explicitamente as variáveis que deveriam ser privadas, sendo elas as que envolviam operações de atribuição, e as variáveis compartilhadas, sendo elas as que envolviam apenas leitura.

O tipo de escalonamento foi estático, cada thread responsável por desenhar um determinado bloco de linhas da imagem final (o loop externo foi o paralelizado).

A segunda diretiva dizia respeito às operações de IO envolvendo o desenho de fato dos pixels na janela do X11:

```c++
#pragma omp critical
{
    XDrawPoint(display, win, gc, j, i);
}
```

Ela apenas garante que essas operações sejam feitas uma thread por vez, para evitar problemas de concorrência.

Antes de começar os testes, uma última mudança foi feita: o número de threads é recebido pela entrada padrão para evitar a necessidade de recompilar o programa para cada teste. A compilação foi feita com o comando `gcc -fopenmp -o mandelbrot mandelbrot.c -lX11 -lm`.

### Medição de desempenho

![image](../images/mandelbrot_efficience.png)

![image](../images/Speedup_threads_q2.png)

É possível notar que há o speed-up até o uso de 16 threads. Utilizando 32 threads, vemos que a performance fica similar, piorando um pouco em certas tentativas. E, com 64 threads, a performance piora significamente, similar a utilizar 8 threads.

Isso ocorre por causa da parte crítica do programa. O aumento de threads faz com que haja muita concorrencia entre o uso dessa área. Uma maneira de contornar essa limitação é realizar o desenho separadamente, e só guardar os valores em uma matriz ou vetor.