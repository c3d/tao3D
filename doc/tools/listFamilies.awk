BEGIN {
}
function extractStr(s){
    if (match( s, "\"[^\"]*\"") > 0)
    {
        return substr( s, RSTART+1, RLENGTH-2)
    }
    return s
}

/(docname )/ {
    familyL = extractStr($3)
    split(familyL, familyA,":")
    for (f in familyA)
        familiesTable[familyA[f]] += 1
#    printf "family %i \n",familiesTable[family]
}

function genToc(f, Nb){
    toc = sprintf( "%s\n%s%s%s%s%s%s%i%s%s%s%s", 
                   toc,
                   "    active_widget\n",
                   "        on_click\n",
                   "            goto_page \"",f,"\"\n",
                   "        text_box 0, 500 - 25*",Nb,", 300, 25, do\n",
                   "            text \"",f,"\"")

}

END {

    i=0
    for (f in familiesTable)
    {
        printf "family %s = %i\n", f, familiesTable[f]
        genToc(f, i++)
        body = sprintf ("%sgroupname \"%s\"\n", body, f )
    }
# Generate all_doc.ddd that should call groupname for each family
   # TAODOC = "./tao_doc.ddd"
    printf "page \"TOC\", do\n" > TAODOC
    printf "%s\n\n%s\n", toc, body >> TAODOC

# Generate html by using doxygen for each family
}
