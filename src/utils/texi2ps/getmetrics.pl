#!/bin/perl5
# -*- perl -*-

sub get_one {
    local($font, $filename, $symbol) = @_;

    print "$filename $font $symbol\n";

    open(OUT, ">$filename");
    print OUT "float $symbol\[] = {\n";
    print OUT "0," x 31, "0\n";
    open(GS, "gs -q -sDEVICE=pcx16 -sthe_font=$font metrics.psc |");
    print OUT while <GS>;
    print OUT "};\n";
    close(OUT);
    close(GS);
}

&get_one("Times-Roman", "tms.h", "fm_tms");
&get_one("Times-Bold", "tmsb.h", "fm_tmsb");
&get_one("Times-Italic", "tmsi.h", "fm_tmsi");
&get_one("Times-BoldItalic", "tmsbi.h", "fm_tmsbi");

&get_one("Helvetica", "hlv.h", "fm_hlv");
&get_one("Helvetica-Bold", "hlvb.h", "fm_hlvb");
&get_one("Helvetica-Oblique", "hlvi.h", "fm_hlvi");
&get_one("Helvetica-BoldOblique", "hlvbi.h", "fm_hlvbi");

&get_one("Courier", "cou.h", "fm_cou");
&get_one("Courier-Bold", "coub.h", "fm_coub");
&get_one("Courier-Oblique", "coui.h", "fm_coui");
&get_one("Courier-BoldOblique", "coubi.h", "fm_coubi");

&get_one("Symbol", "sym.h", "fm_sym");

&get_one("Zapf-Dingbats", "ding.h", "fm_ding");
