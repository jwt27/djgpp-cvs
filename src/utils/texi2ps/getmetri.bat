@echo off

call getmet1 Times-Roman tms.h fm_tms
call getmet1 Times-Bold tmsb.h fm_tmsb
call getmet1 Times-Italic tmsi.h fm_tmsi
call getmet1 Times-BoldItalic tmsbi.h fm_tmsbi

call getmet1 Helvetica hlv.h fm_hlv
call getmet1 Helvetica-Bold hlvb.h fm_hlvb
call getmet1 Helvetica-Oblique hlvi.h fm_hlvi
call getmet1 Helvetica-BoldOblique hlvbi.h fm_hlvbi

call getmet1 Courier cou.h fm_cou
call getmet1 Courier-Bold coub.h fm_coub
call getmet1 Courier-Oblique coui.h fm_coui
call getmet1 Courier-BoldOblique coubi.h fm_coubi

call getmet1 Symbol sym.h fm_sym

call getmet1 Zapf-Dingbats ding.h fm_ding
