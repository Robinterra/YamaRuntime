#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 *
 * @date 2020.12.21
 */

// -----------------------------------------------

#pragma region get/set

// -----------------------------------------------

unsigned int Registers[16];

// -----------------------------------------------

unsigned int MemorySize = 0x2FAF080;

// -----------------------------------------------

unsigned char Memory[0x2FAF080];

// -----------------------------------------------

char Carry = 0;

// -----------------------------------------------

char Zero = 0;

// -----------------------------------------------

unsigned char Command;

// -----------------------------------------------

unsigned char Condition;

// -----------------------------------------------

unsigned int A;

// -----------------------------------------------

unsigned int B;

// -----------------------------------------------

unsigned int C;

// -----------------------------------------------

char IsRun;

// -----------------------------------------------

FILE *fd_int;

// -----------------------------------------------

void *Commands[0x100];

// -----------------------------------------------

#pragma endregion get/set

// -----------------------------------------------

#pragma region Formats

// -----------------------------------------------

int Format3(unsigned char one, unsigned char two, unsigned char three)
{
    Condition =  (one >> 4) & 0xff;
    A =  one & 0x0f;
    B = 0;
    C =  (two << 8) | three;

    return 1;
}

int Format2(unsigned char one, unsigned char two, unsigned char three)
{
    Condition =  (one >> 4) & 0xff;
    A =  one & 0x0f;
    B = ( two & 0xf0) >> 4;
    C =  ((two & 0x0F) << 8) | three;

    return 1;
}

int Format1(unsigned char one, unsigned char two, unsigned char three)
{
    Condition =  (one >> 4) & 0xff;
    A =  one & 0x0f;
    B = ( two & 0xf0) >> 4;
    C =  two & 0x0f;

    return 1;
}

// -----------------------------------------------

#pragma endregion Formats

// -----------------------------------------------

int CheckCondition()
{
    if (Condition == 0) return 1;
    if (Condition == 1) return Zero;
    if (Condition == 2) return !Zero;
    if (Condition == 3) return (!Zero) && (!Carry);
    if (Condition == 4) return (Zero) || (!Carry);
    if (Condition == 5) return (!Zero) && (Carry);
    if (Condition == 6) return (Zero) || (Carry);
    if (Condition == 15) Registers[15] += 4;

    return 1;
}

// -----------------------------------------------

int RunCommand()
{
    if (!CheckCondition()) return 1;

    void (*fun_ptr)() = Commands[Command];

    if (fun_ptr == 0)
    {
        printf("Not Supported\n");
        return 0;
    }

    (*fun_ptr)();

    /*foreach (ICommand command in Commands)
    {
        if (command.CommandId != cmd) continue;

        return command.Execute(this);
    }*/

    //Console.WriteLine("Warning Command {0:x} not found", cmd);

    return 1;
}

// -----------------------------------------------


int ChooseCorrectFormat(unsigned char command, unsigned char one, unsigned char two, unsigned char three)
{
    unsigned char format = (command >> 4);
    if (format == 5) return Format1(one, two, three);
    if (format == 1) return Format2(one, two, three);
    if (format == 2) return Format3(one, two, three);
    if (format == 4) return Format3(one, two, three);
    if (format == 0xf) return Format1(one, two, three);
    if (command == 0x32) return Format3(one, two, three);
    if (command == 0x31) return Format1(one, two, three);
    if (command == 0x30) return Format1(one, two, three);

    return 0;
}

int MakeCommand()
{
    unsigned int adresse = Registers[15];
    unsigned char three = Memory[adresse];
    unsigned char two = Memory[adresse + 1];
    unsigned char one = Memory[adresse + 2];
    unsigned char command = Memory[adresse + 3];
    Command = command;
    /*printf("%x,", command);
    printf("%x,", Registers[1]);
    printf("%x", Registers[12]);
    printf(",%x,", adresse);
    int number;
    scanf("%d", &number);*/

    //printf("r0=%x,r1=%xr2=%x,r3=%x,r4=%x,r9=%x,r10=%xr11=%x,r12=%x,sp=%x,lr=%x,pc=%x,c=%x,a=%x,b=%x,c%x,con=%x\n",Registers[0],Registers[1],Registers[2],Registers[3],Registers[4],Registers[9],Registers[10],Registers[11],Registers[12],Registers[13], Registers[14], Registers[15],Command, A,B,C, Condition);

    return ChooseCorrectFormat(command, one, two, three);
}

// -----------------------------------------------

int EndPhase()
{
    //printf("%x", Registers[15]);
    Registers[15] += 4;
    //printf(",%x,", Registers[15]);

    return 1;
}

// -----------------------------------------------

int Run (  )
{
    IsRun = 1;

    while (IsRun)
    {
        MakeCommand();

        RunCommand();

        EndPhase();
    }

    return Registers[12];
}

// -----------------------------------------------

int ReadFile (  )
{
    fseek(fd_int, 0L, SEEK_END);
    unsigned int filesize = ftell(fd_int);
    rewind(fd_int);

    Registers[15] = 0;
    Registers[13] = MemorySize - 4;
    Registers[12] = filesize;

    if ( 1 != fread( &Memory , filesize, 1 , fd_int) ) exit(1);

    fclose (fd_int);

    return 1;
}

// -----------------------------------------------

#pragma region Commands

// -----------------------------------------------

void AdcRegisterCommand()
{
    unsigned long result = Registers[B] + Registers[C] + Carry;

    Carry = result == 0x100000000;
    result = result & 0xFFFFFFFF;

    Registers[A] = result;
}

// -----------------------------------------------

void AddImediateCommand()
{
    unsigned long result = Registers[B] + C;

    Carry = result == 0x100000000;
    result = result & 0xFFFFFFFF;

    Registers[A] = result;
}

// -----------------------------------------------


void AddRegisterCommand()
{
    unsigned long result = Registers[B] + Registers[C];

    Carry = result == 0x100000000;
    result = result & 0xFFFFFFFF;

    Registers[A] = result;
}

// -----------------------------------------------


void AndRegisterCommand()
{
    Registers[A] = Registers[B] & Registers[C];
}

// -----------------------------------------------

void AslImediateCommand()
{
    Registers[A] = Registers[B] << C;
}

// -----------------------------------------------

void AslRegisterCommand()
{
    Registers[A] = Registers[B] << Registers[C];
}

// -----------------------------------------------

void ASRImediateCommand()
{
    Registers[A] = Registers[B] >> C;
}

// -----------------------------------------------

void ASRRegisterCommand()
{
    Registers[A] = Registers[B] >> Registers[C];
}

// -----------------------------------------------

void BConditionCommand()
{
    int isnotpositive = (C & 0x8000);

    if (isnotpositive) C = C | 0xFFFF0000;

    C = C << 2;

    Registers[15] += C;
}

// -----------------------------------------------

void BxRegisterCommand()
{
    Registers[15] = Registers[A];
}

// -----------------------------------------------

void BlxRegisterCommand()
{
    Registers[14] = Registers[15];
    Registers[15] = Registers[A];
}

// -----------------------------------------------

void CmpImediateCommand()
{
    long cmpresult = Registers[A] - C;

    Carry = cmpresult < 0 ? 1 : 0;
    Zero = cmpresult == 0 ? 1 : 0;
}

// -----------------------------------------------

void CmpRegisterCommand()
{
    long cmpresult = Registers[A] - Registers[B];

    Carry = cmpresult < 0 ? 1 : 0;
    Zero = cmpresult == 0 ? 1 : 0;
}

// -----------------------------------------------

void DivRegisterCommand()
{
    Registers[A] = Registers[B] / Registers[C];
}

// -----------------------------------------------

void EorRegisterCommand()
{
    Registers[A] = Registers[B] ^ Registers[C];
}

// -----------------------------------------------

char* GetStringFromRegister(int reg)
{
    unsigned int adresse = Registers[reg];

    unsigned int length = Memory[adresse];
    length = length | (Memory[adresse + 1] << 8);
    length = length | (Memory[adresse + 2] << 16);
    length = length | (Memory[adresse + 3] << 24);

    char * result = malloc(length + 1);
    result[length] = 0;

    strncpy(result, ((char *)&Memory) + adresse + 4, length);

    return result;
}

// -----------------------------------------------

void ExecRegisterCommand()
{
    unsigned int subcmd = Registers[A];
    if (subcmd == 1)
    {
        printf("%d", Registers[1]);
        return;
    }

    if (subcmd == 2)
    {
        int number = 0;
        scanf("%d", &number);
        Registers[12] = number;

        char *line = NULL;  /* forces getline to allocate with malloc */
        size_t len = 0;     /* ignored when line = NULL */
        ssize_t read;
        read = getline(&line, &len, stdin);
        free(line);

        return;
    }

    if (subcmd == 3)
    {
        //printf("exit with %d\n", Registers[1]);

        exit (Registers[1]);
        return;
    }

    if (subcmd == 4)
    {
        char *textPointer = GetStringFromRegister(1);

        printf(textPointer);

        free (textPointer);

        return;
    }

    if (subcmd == 5)
    {
        char *line = NULL;  /* forces getline to allocate with malloc */
        size_t len = 0;     /* ignored when line = NULL */
        ssize_t read;
        read = getline(&line, &len, stdin);

        //strncpy(result, (&Memory) + adresse + 4, length);

        free (line);
    }
}

// -----------------------------------------------

void LdrCommand()
{
    unsigned int adresse = Registers[B] + (C << 2);
    //printf(",%x ( %x, %x )", adresse, Registers[B], C);
    if (adresse > MemorySize)
    {
        printf("out of bounds");
        exit (1);
    }
    unsigned int data = Memory[adresse];
    data = data | (Memory[adresse + 1] << 8);
    data = data | (Memory[adresse + 2] << 16);
    data = data | (Memory[adresse + 3] << 24);
    //printf(",%x-", data);

    Registers[A] = data;
}

// -----------------------------------------------

void MovImediateCommand()
{
    Registers[A] = C;
}

// -----------------------------------------------

void MovRegisterCommand()
{
    Registers[A] = Registers[B];
}

// -----------------------------------------------

void MulRegisterCommand()
{
    Registers[A] = Registers[B] * Registers[C];
}

// -----------------------------------------------

void OrrRegisterCommand()
{
    Registers[A] = Registers[B] | Registers[C];
}

// -----------------------------------------------

void PopCommand()
{
    for (int i = 15; i >= 0; i--)
    {
        if (((C >> i) & 0x1) == 0) continue;

        Registers[13] += 4;

        unsigned int adresse = Registers[13];

        unsigned int data = Memory[adresse];
        data = data | (Memory[adresse + 1] << 8);
        data = data | (Memory[adresse + 2] << 16);
        data = data | (Memory[adresse + 3] << 24);

        Registers[i] = data;
    }
}

// -----------------------------------------------

void PushCommand()
{

    for (int i = 0; i < 16; i++)
    {

        if (((C >> i) & 0x1) == 0) continue;

        unsigned int adresse = Registers[13];

        unsigned int data = Registers[i];

        Memory[adresse] = data & 0xFF;
        Memory[adresse + 1] = (data & 0xFF00) >> 8;
        Memory[adresse + 2] = (data & 0xFF0000) >> 16;
        Memory[adresse + 3] = (data & 0xFF000000) >> 24;

        Registers[13] -= 4;
    }

}

// -----------------------------------------------

void StrCommand()
{
    unsigned int adresse = Registers[B] + (C << 2);

    unsigned int data = Registers[A];

    Memory[adresse] = data & 0xFF;
    Memory[adresse + 1] = (data & 0xFF00) >> 8;
    Memory[adresse + 2] = (data & 0xFF0000) >> 16;
    Memory[adresse + 3] = (data & 0xFF000000) >> 24;
}

// -----------------------------------------------

void SubImediateCommand()
{
    Registers[A] = (~C) + 1 + Registers[B];
}

// -----------------------------------------------


void SubRegisterCommand()
{
    Registers[A] = (~Registers[C]) + 1 + Registers[B];
}

// -----------------------------------------------

#pragma endregion Commands

// -----------------------------------------------

#pragma region main

// -----------------------------------------------

int InitCommands()
{
    Carry = 0;
    Command = 0;
    A = 0;
    B = 0;
    C = 0;
    Zero = 0;
    
    Commands[0x51] = &AdcRegisterCommand;
    Commands[0x10] = &AddImediateCommand;
    Commands[0x50] = &AddRegisterCommand;
    Commands[0x56] = &AndRegisterCommand;
    Commands[0x1A] = &AslImediateCommand;
    Commands[0x5A] = &AslRegisterCommand;
    Commands[0x19] = &ASRImediateCommand;
    Commands[0x59] = &ASRRegisterCommand;
    Commands[0x32] = &BConditionCommand;
    Commands[0x30] = &BxRegisterCommand;
    Commands[0x31] = &BlxRegisterCommand;
    Commands[0x2F] = &CmpImediateCommand;
    Commands[0x5F] = &CmpRegisterCommand;
    Commands[0x55] = &DivRegisterCommand;
    Commands[0x57] = &EorRegisterCommand;
    Commands[0xFF] = &ExecRegisterCommand;
    Commands[0x1B] = &LdrCommand;
    Commands[0x2E] = &MovImediateCommand;
    Commands[0x5E] = &MovRegisterCommand;
    Commands[0x54] = &MulRegisterCommand;
    Commands[0x58] = &OrrRegisterCommand;
    Commands[0x41] = &PopCommand;
    Commands[0x40] = &PushCommand;
    Commands[0x1C] = &StrCommand;
    Commands[0x12] = &SubImediateCommand;
    Commands[0x52] = &SubRegisterCommand;

    return 1;
}

// -----------------------------------------------

 /**
  * Dies ist der Einstiegpunkt, die Main Funktion
  *
  * @param[in] args_Length_int (int) Die Anzahl der uebergeben Argumente
  * @param[in] *args_chars[] (char) Die Argumente, die dieser Anwendung uebergeben wurden
  *
  * @return int Der (exitcode)Returncode fuer diese Anwendung
  */
int main(int args_Length_int,char *args_chars[])
{
    InitCommands();

    fd_int = fopen(args_chars[1],"r");//open(args_chars[1], O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR);

    if( !fd_int ) printf("file not found"),exit(1);

    ReadFile();

    return Run ();
}

// -----------------------------------------------

#pragma endregion main

// -----------------------------------------------