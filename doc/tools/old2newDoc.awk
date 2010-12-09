BEGIN { 
    input_type = "" # would be INFIX PREFIX POSTFIX BLOCK
    name = ""
    rtype = ""
    symbol = ""
    code = ""
#    parm_type
#    parm_name
#    parm_desc
    b_open = ""
    b_type = ""
    b_close = ""
    t1 = ""
    t2 = ""
    group = "formula"
    syno = ""
    desc = ""
    ret = ""
    misc = ""
    nb_parm = 0
    in_description = 0;

    filename = "formula2_test.tbl"
    printf "/*start*/\n" > filename
} 

function reset() {
    input_type = "" # would be INFIX PREFIX POSTFIX BLOCK
    name = ""
    rtype = ""
    symbol = ""
    code = ""
    delete parm_type
    delete parm_name
    delete parm_desc
    b_open = ""
    b_type = ""
    b_close = ""
    t1 = ""
    t2 = ""
    group = "formula"
    syno = ""
    desc = ""
    ret = ""
    misc = ""
    nb_parm = 0

    in_description = 0;
}

function write_def(){
    parm_def = ""
    # building the beginning of the definition
    beg_of_def = sprintf("%s(%s, %s,", input_type, name, rtype)
    # building parameter list
    l =  length(parm_type)
    if (l > 0)
        parm_def = sprintf("%s\n", parm_def)
    for (i = 1; i <= l; i++)
    {
        parm_def = sprintf("%s    PARM(%s,%s, %s)\n",
                           parm_def, parm_name[i], parm_type[i], parm_desc[i])
    }
    # building the end of the definition
    end_of_def = sprintf("\n    DOC_GROUP(%s),\n    DOC_SYNOPSIS(\"%s\"),\n    DOC_DESCRIPTION(\"%s\"),", 
                         group, syno, desc)
    if (length(ret) != 0)
        end_of_def = sprintf("%s\n    DOC_RET(%s, %s)", end_of_def, rtype, ret )

    end_of_def = sprintf("%s, ", end_of_def)

    if (length(misc) != 0)
        end_of_def = sprintf("%s\n    DOC_MISC(\"%s\")", end_of_def, misc)
    
    if (match(input_type , "PREFIX"))
    {
        printf "%s %s, %s, %s, %s)\n", beg_of_def, symbol, parm_def, code, end_of_def  >> filename
    }
    else if (match(input_type , "POSTFIX"))
    {
        printf "%s %s, %s, %s, %s)\n", beg_of_def, parm_def, symbol, code, end_of_def  >> filename
    }
    else if (match(input_type , "INFIX"))
    {
        printf "%s %s,%s,%s,%s,%s)\n", beg_of_def, t1, symbol, t2, code, end_of_def  >> filename
    }
    else if (match(input_type , "BLOCK"))
    {
        printf "%s %s, %s, %s, %s, %s)\n", beg_of_def, b_open, b_type, b_close, code, end_of_def  >> filename
    }
}

function printAndReset()
{
    if (length(input_type) > 0)
        write_def()
    reset();
}
#PREFIX(Page, text, "page", PARM(n, text) PARM(b, tree), RTAO(page(self,n,b)),
#       "/*| docname \"page\", \"page\", do { \
#                dsyntax <<page name, content>>;\
#                synopsis \"Create a new page.\" ; \
#                description <<Create a new page in the document with the specified name and content.>> ;\
#                parameters;\
#                parameter \"text\", \"name\", <<The name of the page. [TODO ddd: What's happen if the name is not unique in the document ?]>>;\
#                parameter \"tree\", \"content\", <<The content of the page.>>;\
#                return_value \"text\", <<The name of the previously named page>>;\
#                }\
#          |*/")


function extractStr(s){
    if (match( s, "<<[^>]*>>") > 0)
    {
        return substr( s, RSTART+2, RLENGTH-4)
    }

    if (match( s, "\"[^\"]*\"") > 0)
    {
        return substr( s, RSTART+1, RLENGTH-3)
    }
    printf "\t PAS DE MATCH %i\n%s\n", NR, s
    return s
}

# Matches every line
{

    if ( in_description == 1 )
    {
        if (index($0, ">>") > 0)
        {
            desc = sprintf("%s%s", desc, $0)
            in_description = 0
            desc =  sprintf("%s", extractStr(desc))
#            printf "|%s|\n\n", desc
        }
        else
        {
            # removes the \ at the end of the line
            desc = sprintf("%s%s\n", desc, $0)
        }
        next
    }
}

/NAME|TYPE/ {
    printf "%s\n", $0 >> filename
}

/PREFIX|POSTFIX|INFIX|BLOCK/ {
    printAndReset()
#    printf "DEB: matches PREFIX|POSTFIX|INFIX|BLOCK\n|%s|\n\n", $0
    ei = index($0,"(")
    input_type = substr ( $0, 1, ei-1);
    line = substr($0, ei+1, length - ei)
    split(line, par, ",")
    name = par[1]
    rtype = par[2]
    if (match ( input_type, "PREFIX"))
       symbol = par[3]
    if (match ( input_type, "POSTFIX"))
       symbol = par[5]
    if (match ( input_type, "INFIX"))
    {
        t1     = par[3]
        symbol = par[4]
        t2     = par[5]
    }
    printf "DEB:input_type = |%s| t1 = |%s| symbol = |%s| t2 = |%s|\n\n", input_type, t1, symbol, t2
 
}

/PARM/{
#    printf "DEB: matches PARM \n|%s|\n", $0
    line = $0
    
    while ( 0 != (bi = index(line, "PARM(")))
    {
        bi += 5
        nb_parm++
        line = substr( line, bi, length(line))
        ei = index(line, ")")
        parm_couple = substr(line, 1, ei-1)
        split(parm_couple, params , "," )
        parm_type[nb_parm] = params[2]
        parm_name[nb_parm] = params[1]
#        printf "DEB:%i param name = |%s| type = |%s|\n",nb_parm, parm_name[nb_parm] , parm_type[nb_parm]
        line = substr(line,ei, length(line))
#        printf "DEB: fin de la ligne |%s|\n\n", line
    }
}

/RTAO/{
     bi = index($0, "RTAO")
     line = substr($0, bi, length)
     ei = index(line, ")),")
     code = substr(line, 1, ei+1)
#     printf "DEB: code %i = |%s|\n", NR, code
}
/XL_/{
     bi = index($0, "XL_")
     line = substr($0, bi, length)
     ei = index(line, ")),")
     l = 1
     if (ei <= 0 )
     {
         ei = index(line, "),")
         l = 0
     }
     code = substr(line, 1, ei+l)
#     printf "DEB: code %i = |%s|\n", NR, code
}
/return/{
     bi = index($0, "return")
     line = substr($0, bi, length)
     ei = index(line, "),")
     code = substr(line, 1, ei)
#     printf "DEB: code %i = |%s|\n", NR, code
}

/(  docname )/ {
#    printf "DEB:  match docname\n|%s|\n", $0
    nb_parm = 0

    nb_word = split( $0, words, "\\\\\"")
    group = words[4]
#    printf "group = |%s|\n\n", group
}

/(dsyntax )/ { 
   
}

/(  synopsis )/ {
    
    syno = sprintf("%s", extractStr($0))
#    printf "DEB: matches synopsis\n|%s|\n|%s|\n\n",  $0, syno
}

/(  description )/ {
    if (index($0, ">>") > 0 || index($0, "\"") > 0)
    {
        desc =  sprintf("%s", extractStr($0))
        in_description = 0
    }
    else
    {
        desc = sprintf("%s\n",$0)
        in_description = 1
    }
#    printf "DEB: matches descriptoin\n|%s|\n|%s|\n\n",  $0, desc

}

/>>/ {
    if (in_descrition == 1)
    {
        desc =  sprintf("\"%s\"", extractStr(desc))
#        printf "|%s|\n\n", desc
    }
    in_description = 0
}
/(  parameters)/ {

}

/(  parameter )/ {
    nb_parm++
    bi = index($0, "<<")
    ei = index($0, ">>")

    parm_desc[nb_parm] = sprintf("\"%s\"", substr($0, bi+2, ei - bi-2))

#    printf " param %s %s %s \n", parm_name[nb_parm], parm_type[nb_parm], parm_desc[nb_parm]
}


/return_value / {
    bi = index($0, "<<")
    ei = index($0, ">>")

    ret = sprintf("\"%s\"", substr($0, bi+2, ei - bi-2))
}

/(see[ \t]+[\"<]{1,2})/ {
    if (length(misc) > 0)
       misc = sprintf("%s%s\n", misc, $0)
    else
       misc = sprintf("%s\n", $0)
}

/err / {
    if (length(misc) > 0)
       misc = sprintf("%s%s\n", misc, $0)
    else
       misc = sprintf("%s\n", $0)
}

/\|\*\// {
# end of comments
#    printf "FIN de COMMENTAIRE\n\n|%s|\n", $0
    printAndReset();
}

/^\/\// {
    printf "%s\n", $0 >> filename
}


!/[a-z=})\/]/{
     printf "%s\n", $0 >> filename
}

END {
    printAndReset();
    # closes the last definition

#print "BYE "
}
