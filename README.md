# TP3 &mdash; DCC605 File System Checker (DCC-FSCK)
### Bruno Cesar Pimenta Fernandes &mdash; 2013007161 / Nathalia Campos Braga &mdash; 2013069116
* [Repositório, com a documentação atualizada (README.md)](https://github.com/brunocpf/so_tp3)


## Introdução

Este trabalho tem como objetivo a implementação do FSCK para o sistema Extended File System 2 (ext2).<br/>Os computadores estão sujeitos à eventuais problemas, tais como queda de energia, desligamento incorreto ou defeitos físicos no disco rígido, que podem ocasionar no corrompimento de arquivos no sistema de arquivos.<br/>O FSCK é uma ferramenta usada para verificar e corrigir erros em discos e sistemas de arquivos no Linux. Na maioria dos casos o FSCK corrige automaticamente os blocos que apresentam defeitos e os demais problemas. Já em alguns casos em que ocorrem maiores danos, o programa o força a uma checagem manual, realizada diretamente pela linha de comando.<br/>O FSCK implementado neste trabalho tem a função de tratar quatro erros causados ao ext2 que são: 
1. Ataque ao superbloco.
1. Blocos pertecentes a mais de um inode. 
1. Permissões de um arquivo zeradas.
1. Inodes não pertecentes a nenhum diretório.

Como executar o programa:
compile e rode ./dcc_os_fsck path
onde path é o arquivo da imagem ext2 a ser testada.

## Implementação
ext2.h: definições úteis do ext2 copiadas do original (sbloco, inodes, etc)
dcc.os.fsck.c: implementação do fsck

funções/macros auxiliares:

ler o superbloco: (vide especificação)
ler o grupo: (também)
bitmaps: ler o grupo, depois ler os blocos com offset correspondente usando campos do superblock. armazenados em um array de char (1 byte/8 bits por elemento)
testar valor de um bit no bitmap: macro TEST_BIT, usando lógica binária
zerar o valor de um bit no bitmap: macro CLEAR_BIT, usando lógica binária


read/write inode: retirado de http://cs.smith.edu/~nhowe/262/oldlabs/ext2.html
adicionado parametro "group_no" para fazer offset no bloco correspondente

Obs: todas as ações dependem da entrada do usuário
Obs2: os inodes de número < que EXT2_GOOD_OLD_FIRST_INO (11) são ignorados

Fun 1:
-> Verifica se o disco foi atacado usando testando se o magic number bate (is_attacked)
-> Perguntar ao usuário se ele deseja recuperar um backup no disco
-> Para cada endereço de superbloco, ler o bloco e verificar se o magic number bate ----> "For filesystems with 1k blocksizes, a backup superblock can be found at block 8193; for filesystems with 2k blocksizes, at block 16384; and  for  4k blocksizes, at block 32768."
-> O primeiro que bater é copiado no super (recuperado como backup)

Fun 2:
ideia:
-> array owned_blocks mantem um contador para o número de donos de cada bloco
-> percorre sobre todos os inodes do disco, vendo os blocos utilizados e incrementando o contador correspondente
-> percorre sobre owned_blocks e vê se algum contador está maior que 1, se sim percorre novamente os inodes, verifica quais estão usando este bloco, seta para zero o bit do i-map (bitmap de inodes) do primeiro inode encontrado, deletando este
-> vê novamente se sobrou algum bloco com mais de um dono, e se sim chama a função recursivamente até não sobrar nenhum.

Fun 3:
-> assim como no fun 2, percorre todos os inodes.
-> verifica se a permissão é inválida ((inode.i_mode & 0xff) == 0)
-> se sim, pede para o usuário entrar com o número decimal correspondente à permissão que ele deseja (formato chmod)
-> modifica a permissão com a função write_inode

## Testes

Para a realização de testes, além do script disponibilizado, foram criadas imagens utilizando os comandos dd e mkfs. Para introduzir erros nessas imagens manualmente, foi usado o comando debugfs.

## Conclusão

O trabalho possibilitou o maior aprendizado sobre como o sistema Extended File System 2 (ext2) funciona e como o FSCK pode ser implementado para atuar, tratando possíveis erros causados ao ext2. 
O trabalho apresentou o resultado esperado, passando em todos os testes realizados.

## Referências

http://cs.smith.edu/~nhowe/262/oldlabs/ext2.html<br/>http://pages.cs.wisc.edu/~remzi/OSTEP/file-ffs.pdf<br/>http://pages.cs.wisc.edu/~remzi/OSTEP/file-journaling.pdf
