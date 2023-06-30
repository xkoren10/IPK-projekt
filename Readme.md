# IPK projekt 1, Matej Koreň [xkoren10]

Server v jazyku C/C++ komunikujúci prostredníctvom protokolu HTTP, poskytujúci rôzné informácie o systéme.

## Spustenie

Projekt je potrebné preložiť pomocou priloženého makefile-u príkazom 
$make
Následné spustenie serveru je vo formáte príkazu
```
$./hinfosvc port
```
prípadne
```
$./hinfosvc port &
```

kde *port* je číslo portu, na ktorom server príma požiadavky. Pri nesprávnom formáte sa program ukončí s chybovou hláškou. 

Po správnom spustení je server spustený a čaká na požiadavku zo stany klienta. Dokáže odpovedať na nasledujúce požiadavky:

```
$ curl http://localhost:port/hostname  -> vráti doménové meno 
```
```
$ curl http://localhost:port/cpu-name  -> vráti názov CPU
```
```
$ curl http://localhost:port/load  -> vráti momentálnu záťaž procesoru
```

Na ostatné požiadavky sserver odpovedá chybovou hláškou *400 Bad Request*

## Príklad použitia

*********/Počítač/ME/IPK$ make
```
$gcc -std=gnu99 -Wall -Wextra -pedantic -lm -o hinfosvc hinfosvc.c -lm
```
*********/Počítač/ME/IPK$ ./hinfosvc 1234 &
```
$[1] 9532
```
*********/Počítač/ME/IPK$ curl http://localhost:1234/load
```
$7%
```
*********/Počítač/ME/IPK$ curl http://localhost:1234/hostname
```
$Acerus-Aspirus
```
*********/Počítač/ME/IPK$ curl http://localhost:1234/cpu-name
```
$Intel(R) Core(TM) i7-10510U CPU @ 1.80GHz
```
********/Počítač/ME/IPK$ kill 9532
********/Počítač/ME/IPK$ make clean
```
$rm -f *.o hinfosvc
$[1]+  Terminated              ./hinfosvc 1234
```

## Zhrnutie implementácie

Projekt využíva knižnice na prácu so socketmi a sieťou. Okrem hlavnej funkcie obsahuje aj dve pomocné - isNumeric(), ktorá sa používa pri validácii argumentu, t.j. zadaného portu a funkcia cpusage(), ktorá bola inšpirovaná vláknom 
[Accurately calculating cpu utilization in linux using /proc/stat](https://stackoverflow.com/questions/5514119/accurately-calculating-cpu-utilization-in-linux-using-proc-stat)

V hlavnej funkcií sa najprv otestujú zadané argumenty, do pomocných bufferov sa načítajú statické údaje o serveri (hostname,cpu-name) a vytvorí sa socket.
Následne sa nastavia paramere potrebné pre konfiguráciu hlavného a komunikačného socketu, bindingu a príjmaniu požiadavok. Funkcia setsockopt() nám pomôže využívať rovnaký port a adresu po výpadku alebo ukočení činnosti automaticky. bind() spáruje nastaví socketu adresu a listen() pripraví socket naslúchať na konkrétnom zadanom porte.
Hlavná funkcionalita beží v nekonečnom cykle, kde socket čaká na požiadavku od klienta, ktorá môže byť typu load,cpu-name,hostname alebo neplatná. Pri pripojení na server sa znova kvôli efektivite cyklí a číta požiadavky klienta do buffferu. Na základe požiadavky (po prečítaní bufferu) sa vo všetkých prípadoch sa cez send() odosiela http hlavička (platná alebo neplatná) a adekvátna odpoveď predom nahraná v pomocnom zásobníku.

## Poznámky

* Pri spustení s argumentom '&' server beží ako proces na pozadí -> ukončí sa príkazom 
```
$ kill PID
```
kde PID je číslo spusteného procesu.

* Po ukončení programu je možné vymazať objektové súbory príkazom
```
$ make clean
```
