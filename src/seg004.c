/* seg004: EMS (LIM expanded memory) helpers */

extern int ems_other_page;

int sub_18953(int page, int log)
{
    _asm {
        mov ah, 44h
        mov al, byte ptr log
        mov bx, page
        mov dx, ems_other_page
        int 67h
    }
}

int sub_1896B(void)
{
    if (ems_other_page == -1)
        return;
    _asm {
        mov ah, 45h
        mov dx, ems_other_page
        int 67h
    }
}

int sub_18A64(void)
{
    _asm {
        mov ah, 47h
        mov dx, ems_other_page
        int 67h
        mov al, ah
        mov ah, 0
    }
}
