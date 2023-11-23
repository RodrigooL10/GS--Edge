# GS - EDGE COMPUTING & COMPUTER SYSTEMS 

## Problema abordado
O projeto aborda o desafio significativo de monitorar o sono de maneira eficaz, uma vez que os métodos convencionais são geralmente desconfortáveis e trabalhosos, resultando em informações imprecisas devido à relutância das pessoas em utilizá-los. Essa falta de compreensão dos padrões de sono afeta adversamente a saúde mental e física, levando a subestimação ou superestimação de problemas relacionados ao sono. Os impactos incluem riscos como acidentes de trânsito, erros no trabalho e queda de produtividade devido à sonolência diurna. Resolver esse problema é crucial não apenas por uma questão de comodidade, mas fundamental para promover a saúde e o bem-estar, destacando a necessidade de desenvolver tecnologias acessíveis e não invasivas.


## Solução proposta
O nosso projeto propõe uma mudança na forma como monitoramos a qualidade do sono, introduzindo um aplicativo que utiliza sensores integrados a dispositivos comuns, como smartphones e dispositivos vestíveis, para fornecer uma análise mais precisa e acessível dos padrões de sono. Reconhecendo as limitações dos métodos tradicionais, o aplicativo captura dados em tempo real sobre o sono do usuário, medindo vários aspectos, como padrões de movimento, frequência cardíaca e distúrbios respiratórios. Essa abordagem baseada em sensores busca eliminar as barreiras associadas aos métodos convencionais, buscando não apenas melhorar a precisão das informações sobre o sono, mas também tornar a tecnologia acessível e fácil de adotar para diversos usuários. O aplicativo representa um avanço significativo em direção a um monitoramento do sono mais eficiente e integrado às tecnologias do cotidiano.




## Instruções para uso
- O sensor de pulsação estará ativado a todo momento, para identificar quando o usuário dormir.
- Assim que o sensor detectar 60BPM ou menos os demais sensores (MPU6050 e LDR) serão ativados para começar o monitoramento do sono.
- O sensor MPU6050 detectará quando movimentos 'bruscos', ou seja, que interfiram no sono acontecerem.
- O sensor LDR detectará se a luminosidade do ambiente está adequada para uma boa noite de sono.
- Esses dados são enviados para o servidor e serão analisados posteriormente.



## Link do Simulador Wokwi:
https://wokwi.com/projects/382052017085185025



## Integrantes:
- Eric Rodrigues - RM 550249
- Rodrigo Lima - RM 98326
