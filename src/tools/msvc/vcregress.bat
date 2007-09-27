@echo off
REM $PostgreSQL: pgsql/src/tools/msvc/vcregress.bat,v 1.14 2007/08/27 12:10:47 mha Exp $
REM all the logic for this now belongs in vcregress.pl. This file really
REM only exists so you don't have to type "perl vcregress.pl"
REM Resist any temptation to add any logic here.
@perl vcregress.pl %*
