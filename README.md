# TP3 &mdash; DCC605 File System Checker (DCC-FSCK)
### Bruno Cesar Pimenta Fernandes &mdash; 2013007161 / Nathalia Campos Braga &mdash; 2013069116
* [Repositório, com a documentação atualizada (README.md)](https://github.com/brunocpf/so_tp3)


## Introdução

  Este trabalho tem como objetivo a implementação do FSCK para o sistema Extended File System 2 (ext2). 
Os computadores estão sujeitos à eventuais problemas, tais como queda de energia, desligamento incorreto ou defeitos físicos no disco rígido, que podem ocasionar no corrompimento de arquivos no sistema de arquivos. O FSCK é uma ferramenta usada para verificar e corrigir erros em discos e sistemas de arquivos no Linux. Na maioria dos casos o FSCK corrige automaticamente os blocos que apresentam defeitos e os demais problemas. já em alguns casos em que ocorrem maiores danos, o programa o força a uma checagem manual, realizada diretamente pela linha de comando.  
O FSCK implementado neste trabalho tem a função de tratar quatro erros causados ao ext2 que são: 
    1. Ataque ao superbloco.
    2. Blocos pertecentes a mais de um inode. 
    3. Permissões de um arquivo zeradas.
    4. Inodes não pertecentes a nenhum diretório.
  
## Implementação

## Testes

## Extras

## Conclusão

  O trabalho possibilitou o maior aprendizado sobre como o sistema Extended File System 2 (ext2) funciona e como o FSCK pode ser implementado para atuar, tratando possíveis erros causados ao ext2. 
O trabalho apresentou o resultado esperado, passando em todos os testes realizados.

Link(acesso à implementação do trabalho): https://github.com/brunocpf/so_tp3
