BEGIN { 
    previous_syntaxes = ""
    filename = sprintf("c_files/%s.c",FAMILYNAME)
    printf "/**\n * @file %s\n * File detailed description for family %s\n */\n", filename, FAMILYNAME > filename
    OK = 0
    inLongText = 0
    lastLongText = ""
} 
function extractStr(s){
    if (match( s, "<<[^>]*>>") > 0)
    {
        return substr( s, RSTART+2, RLENGTH-4)
    }

    if (match( s, "\"[^\"]*\"") > 0)
    {
        return substr( s, RSTART+1, RLENGTH-2)
    }
    if (match( s, "<<[^>]*") > 0)
    {
        inLongText = 1
        return substr( s, RSTART+2, RLENGTH-2)
    }
    if (match( s, "[^>]*>>") > 0)
    {
        inLongText = 0
        return substr( s, RSTART, RLENGTH-2)
    }

    return s
}
# matches every entry
{
    if (inLongText == 1)
    {
        desc = extractStr($0)
        gsub("\n","<BR>\n",desc)
        printf " * %s\n", desc >> filename
        next
    }
}

/(docname )/ {
#    printf "DEB: NR = %i, NF = %i  matches docname\n|%s|\n\n", NR, NF, $0
    # closes the previous definition
    if ( OK > 0)
    {
        printf " */\n" >> filename
        printf "%s\n", previous_syntaxes >> filename
    }
    # Check if new definition is in the FAMILY
    fn_families = extractStr($3)
    OK = index(fn_families, FAMILYNAME) 
    if (OK > 0)
    {
        # start new definition
        previous_syntaxes = ""
        printf "/**\n" >> filename
    }
}

/(dsyntax )/ { 
    if (OK <=0 ) next
#    printf "DEB: NR = %i, NF = %i  matches dsyntax\n|%s|\n\n", NR, NF, $0
    # dsyntax is not seperated from the docname with a ; so substract the dsyntax line
    i = index( $0, "dsyntax")
    synt = substr( $0, i, length - i +1 )
    n = extractStr(synt)
    
    size = split (n, w)
    myline = sprintf("%s (", w[1])
    for ( j = 2; j <= size; j++ )
    {
        myline = sprintf( "%s %s", myline, w[j])
    }
    myline = sprintf( "%s)", myline)

    printf " * @fn %s\n",myline >> filename
    previous_syntaxes = sprintf ("%s%s;\n", previous_syntaxes, myline)
    
}

/(synopsis )/ {
    if (OK <=0 ) next
    syno = extractStr($0)
#    printf "DEB: NR = %i, NF = %i  matches synopsis\n|%s|\n\n", NR, NF, $0
    printf " * @brief %s\n", syno >> filename
}

/(description )/ {
    if (OK <=0 ) next
#    printf "DEB: NR = %i, NF = %i matches description \n|%s|\n\n", NR, NF, $0
    desc = extractStr($0)
    gsub("\n","<BR>\n",desc)
    printf " *\n * %s\n", desc >> filename
}

/(parameters)/ {
    if (OK <=0 ) next
    printf " *\n" >> filename
}

/(parameter )/ {
    if (OK <=0 ) next
#    printf "DEB: %s %s %s %s\n", $1, $2, $3, $4
    param_name = extractStr($3)
    param_type = extractStr($2)
    split ($0, val, ",")
    param_desc = extractStr(val[3])
    printf " * @param %s [%s] %s\n", param_name, param_type, param_desc >> filename
}

/(see[ \t]+[\"<]{1,2})/ {
    if (OK <=0 ) next
    printf " * @see %s\n", extractStr($0) >> filename
}

/return_value / {
    if (OK <=0 ) next
    printf " * @returns %s\n", extractStr($0) >> filename
}

/err / {
    if (OK <=0 ) next
    printf " * @exception %s\n", extractStr($0) >> filename
}

END {
    # closes the last definition
    if (OK > 0)
    {
        printf " */\n%s\n", previous_syntaxes >> filename
    }
#print "BYE "
}
