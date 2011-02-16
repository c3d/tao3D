# ******************************************************************************
# xlrToDoxygen.awk                                                   Tao project
# ******************************************************************************
# File Description:
# Awk script that generates c files to feed doxygen.
# ******************************************************************************
# This document is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
# (C) 2011 Catherine Burvelle <cathy@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************
#requires -v FAMILYNAME="aFamily"
BEGIN { 
    previous_syntaxes = ""
    #system("mkdir -p c_files")
    filename = sprintf("c_files/%s.c",FAMILYNAME)
    printf "/**\n * @addtogroup %s\n * Detailed description for group %s\n *", FAMILYNAME, FAMILYNAME > filename
    printf "\n * For more information see <a href=\"%s_doc.html\">%s_doc</a>\n * @{\n */\n", FAMILYNAME, FAMILYNAME >> filename
    OK = 0
    inLongText = 0
    lastLongText = ""
} 
function extractStr(s){
    if (match( s, "<<[^>]*>>") > 0)
    {
        s = substr( s, RSTART+2, RLENGTH-4)
        gsub("\n","<BR>\n",s)
        return s
    }

    if (match( s, "\"[^\"]*\"") > 0)
    {
        return substr( s, RSTART+1, RLENGTH-2)
    }
    if (match( s, "<<[^>]*") > 0)
    {
    #printf "DEB: extractString enters long string\n"
        inLongText = 1
        s = substr( s, RSTART+2, RLENGTH-2)
        s= sprintf("%s<BR>",s)
        return s
    }
    if (match( s, "[^>]*>>") > 0)
    {
    #printf "DEB: extractString leaves long string\n"
        inLongText = 0
        s = substr( s, RSTART, RLENGTH-2)
        return s
    }
    #printf "DEB: extractString returns full string\n"
    s = sprintf("%s<BR>",s)
    return s
}
# matches every entry
{
    if (inLongText == 1)
    {
        desc = extractStr($0)
        printf " * %s\n", desc >> filename
        next
    }
}

/(docname )/ {
    #printf "DEB: NR = %i, NF = %i  matches docname\n|%s|\n\n", NR, NF, $0
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
    #printf "DEB: NR = %i, NF = %i  matches dsyntax\n|%s|\n\n", NR, NF, $0
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
    #printf "DEB: NR = %i, NF = %i  matches synopsis\n|%s|\n\n", NR, NF, $0
    printf " * @brief %s\n", syno >> filename
}

/(description )/ {
    if (OK <=0 ) next
    #printf "DEB: NR = %i, NF = %i matches description \n|%s|\n\n", NR, NF, $0
    desc = extractStr($0)
    printf " *\n * %s\n", desc >> filename
}

/(parameters)/ {
    if (OK <=0 ) next
    printf " *\n" >> filename
}

/(parameter )/ {
    if (OK <=0 ) next
    #printf "DEB: NR = %i, NF = %i matches parameter \n|%s|\n", NR, NF, $0
    #printf "DEB: %s %s %s %s\n", $1, $2, $3, $4
    param_name = extractStr($3)
    param_type = extractStr($2)
    split ($0, val, ",")
   # printf "length %i\n", length(val)
    for ( i = 3; i <= length(val); i++)
        param_desc = sprintf("%s%s", param_desc, val[i])
    param_desc = extractStr(param_desc)
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
    printf "/** @} */\n" >> filename
#print "BYE "
}
