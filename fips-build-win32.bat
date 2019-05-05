@echo off

call .\fips clean sapp-win64-vs2017-debug
call .\fips gen sapp-win64-vs2017-debug
call .\fips make viewer-sapp-ui sapp-win64-vs2017-debug
call .\fips run viewer-sapp-ui sapp-win64-vs2017-debug