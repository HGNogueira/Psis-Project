#!/bin/bash

(cd ./Imagens/Animais && exec ./client_add animais.txt &)
(cd ./Imagens/Bandeiras && exec ./client_add bandeiras.txt &)
(cd ./Imagens/Estádios && exec ./client_add estadios.txt &)
(cd ./Imagens/Instrumentos && exec ./client_add instrumentos.txt &)
(cd ./Imagens/LetrasGregas && exec ./client_add letras_gregas.txt &)
(cd ./Imagens/Monumentos && exec ./client_add monumentos.txt)
