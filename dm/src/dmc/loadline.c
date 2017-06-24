/**
 * Implementation of the
 * $(LINK2 http://www.digitalmars.com/download/freecompiler.html, Digital Mars C/C++ Compiler).
 *
 * Copyright:   Copyright (c) 1985-1998 by Symantec, All Rights Reserved
 *              Copyright (c) 2000-2017 by Digital Mars, All Rights Reserved
 * Authors:     $(LINK2 http://www.digitalmars.com, Walter Bright)
 * License:     Distributed under the Boost Software License, Version 1.0.
 *              http://www.boost.org/LICENSE_1_0.txt
 * Source:      https://github.com/DigitalMars/Compiler/blob/master/dm/src/dmc/loadline.c
 */

#include        <stdio.h>
#include        <string.h>
#include        <stdlib.h>
#include        <ctype.h>
#include        "cc.h"
#include        "token.h"
#include        "global.h"
#include        "parser.h"

extern char switch_E;

#if _WIN32

__declspec(naked) void __near inident()
{
    _asm
    {   push    EBX
        mov     EAX,xc                          ;AH is set to 0

        push    EDI
        mov     EBX,offset tok_ident+IDMAX      ;EBX = &tok_ident[IDMAX]

        mov     tok_ident,AL
        mov     EDI,offset tok_ident+1          ;p = tok_ident

        mov     EDX,0x83
        mov     ECX,btextp

        cmp     switch_E,AH
        jne     L2

        even
L1:
#if 1
        ;Relies on IDMAX being even
        mov     AL,[ECX]
        inc     ECX
        mov     [EDI],AL                        ;*p++ = xc
        inc     EDI
        test    byte ptr _chartype[EAX + 1],DL  ;if !isidchar(xc)
        jle     L1A

        mov     AL,[ECX]
        inc     ECX
        mov     [EDI],AL                        ;*p++ = xc
        inc     EDI
        test    byte ptr _chartype[EAX + 1],DL  ;if !isidchar(xc)
        jle     L1A

        cmp     EDI,EBX
        jb      L1
        jmp     Lerr
#else
        mov     AL,[ECX]
        inc     ECX
        mov     [EDI],AL                        ;*p++ = xc
        inc     EDI
        test    byte ptr _chartype[EAX + 1],DL  ;if !isidchar(xc)
        jle     L1A
        cmp     EDI,EBX
        jbe     L1
        jmp     Lerr
#endif

L1A:    lea     EDI,-1[EDI]
        js      L8                              ;0 or 0xFF
        cmp     AL,'\\'
        je      Lspecial
L10:    mov     byte ptr xc,AL
        mov     [EDI],AH                        ;AH is 0 (terminate string)
        mov     btextp,ECX

        ;  idhash = (xc<<16) + ((p - tok_ident) << 8) + p[-1]
        mov     AL,tok_ident
        shl     EAX,16
        mov     AL,-1[EDI]
        sub     EDI,offset tok_ident
        shl     EDI,8
        add     EAX,EDI
        pop     EDI
        pop     EBX
        mov     idhash,EAX
        ret

        even
L2:     mov     AL,[ECX]
        inc     ECX
        test    byte ptr _chartype[EAX + 1],0x83        ;if !isidchar(xc)
        jle     L4
        cmp     switch_E,AH
        jne     L6
L3:     cmp     EDI,EBX
        jae     Lerr
        mov     [EDI],AL                        ;*p++ = xc
        inc     EDI
        jmp     L2

L4:     js      L8                              ;0 or 0xFF?
        cmp     switch_E,AH
        je      L9
L6:     push    EAX
        push    ECX
        push    EAX
        call    explist
        add     ESP,4
        pop     ECX
        pop     EAX
        jmp     L9

L8:     dec     ECX
        mov     btextp,ECX
        call    egchar2
        mov     ECX,btextp

L9:     cmp     AL,'\\'
        je      Lspecial
        test    byte ptr _chartype[EAX + 1],3   ;if !isidchar(xc)
        jz      L10
        jmp     L3

Lerr:   push    ECX
        push    EM_ident2big
        call    lexerr                          ; identifier is too long
        add     ESP,4
        pop     ECX
        xor     EAX,EAX
        sub     EDI,IDMAX - 3
        jmp     L2

Lspecial:
        mov     byte ptr xc,AL
        mov     btextp,ECX
        push    EDI
        call    inidentX
        add     ESP,4
        pop     EDI
        pop     EBX
        ret
    }
}

#endif
