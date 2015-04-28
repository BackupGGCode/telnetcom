# Instrucciones #

Para compilar:

Entras a http://www.boostpro.com/download/
y descargas la ultima versión, compatible con tu pc.
Por ejemplo:
BoostPro 1.51.0 Installer (64-bit) (195K .exe)
Lo instalas.

Luego en el visual vas a
Propiedades del proyecto TelnetCom >>

C/C++ >> Directorios de inclusión adicionales y pones el directorio de instalación del boost, ejemplo: C:\boost\boost\_1\_51.

Luego vas a Vinculador >> Directorio de bibliotecas adicionales y poner la carpeta lib que hay en el directorio de boost,
ejemplo: C:\boost\boost\_1\_51\lib

Guardas y cambias la configuración de Debug a release y ya deberías poder compilar.