## Questão 3.

### Processo de desenvolvimento:

Para paralelizar o método de Jacobi sequencial disponibilizado no AVA, primeiro foi notado que existiam 3 for loops relevantes que podiam ser otimizados com mais threads:

1. O loop de inicialização na função `iniciar()`, que passa por todas as células de A e coloca nelas um valor inicial (definido no problema como 20ºC).
    - _Obs._: No código original, existem mais 2 loops para inicializar as colunas e linhas da borda da matriz, mas estes foram removidos pois o problema apresentado não necessita deles.
2. O loop de cálculo da nova matriz Anew, que passa por cada célula de Anew e calcula seu valor com base na matriz A da iteração anterior.
    - _Obs._: O fato de usarmos apenas a matriz anterior nesse cálculo é justamente o que facilita a paralelização, porque apenas lemos os valores de A.
3. O loop de cálculo do erro e de atribuição de Anew para A, que passa por cada célula da nova matriz, cálcula o erro máximo encontrado, e passa os valores novos de Anew para A.

#### Loop 1

A paralelização desse loop é bem trivial. A seguinte diretiva cuida do seu processo:

```c
#pragma omp parallel for private(j) shared(A) schedule(static)
```

Com isso, usando escalonamento estático, atribuímos um bloco de colunas da matriz para cada thread inicializar separadamente. A variável j é privada para uma thread não interferir nas linhas das outras, enquanto A pode continuar sendo compartilhada (evitando um gasto excessivo de memória) porque cada thread vai mexer em posições na matriz apenas delas, não compartilhando nenhuma tupla (i,j) com nenhuma outra.

#### Loop 2

A lógica nesse loop é bastante parecida com a do anterior, e portanto sua diretiva é bem similar:

```c
#pragma omp parallel for private(j) shared(A, Anew) schedule(static)
```

Novamente, cada thread vai operar no seu bloco de colunas exclusivamente. Tanto A quanto Anew podem ser compartilhadas: Anew é compartilhada porque em momento nenhum uma thread vai invadir a célula de outra para fazer a atribuição - cada célula atribuída é unicamente sua -, enquanto A pode ser compartilhada porque, apesar de uma thread poder "invadir o espaço" de outra por conta dos indíces \[j+1\] e \[j-1\], esses valores são apenas lidos, nunca atribuídos.


#### Loop 3

Esse loop é crucial para o funcionamento do algoritmo e exige um cuidado extra. Sua diretiva é:

```c
#pragma omp parallel for private(j) reduction(max:dt) shared(A, Anew) schedule(static)
```

As cláusulas shared, private e schedule foram escolhidas seguindo lógicas análogas aos laços anteriores, mas aqui foi preciso usar a operação de redução em dt. Isso se dá pois o algoritmo monitora a diferença máxima obtida na última iteração para decidir se vai continuar calculando ou não. Isso significa que não basta para nós manter dt privado dentro de cada thread e jogá-lo fora ao fim da zona paralela de código, porque isso culminaria na permanência do dt da thread mestre inalterado. A solução é simples: dt vira uma variável de redução com operação de máximo, de forma que cada thread vai calcular o seu dt máximo e ao final vai ser feita uma operação de "máximo entre os máximos" para reduzir e obter o verdadeiro dt máximo na zona sequencial do código. Assim, o funcionamento do while continua saudável.

