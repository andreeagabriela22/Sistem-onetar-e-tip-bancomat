# Sistem-onetar-e-tip-bancomat
Se dores, te implementarea unui sistem pentru efectuarea de operat¸iuni bancare. ˆIn cadrul sistemului se
consider˘a existent,a a dou˘a tipuri de entit˘at, i: un server bancar care ofera doua servicii: bancomat-ATM
(pentru interogare sold, retragere numerar, depunere numerar) si serviciul deblocare; s, i client,i care vor
permite utilizatorilor accesarea facilit˘at, ilor oferite de server.
La pornire serverul primes, te dou˘a argumente: un numar de port s, i numele unui fis, iere ce cont, ine datele
client, ilor, a c˘arei structur˘a va fi explicat˘a ulterior (Sectiunea Fisierul de date folosit de server). Modul de
pornire al serverului este:
./server <port_server> <users_data_file>
Un client va primi ca argumente la pornire IP-ul s, i portul serverului. Modul de pornire al clientului este:
./client <IP_server> <port_server>
Codul pentru implementarea serverului s, i al clientului va fi secvent, ial (nu se vor folosi mai multe threaduri)
s, i se va folosi apelul select pentru multiplexarea comunicat, iei (cu entit˘at, ile din sistem s, i/sau interfat,a
utilizator).
