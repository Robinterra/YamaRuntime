#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WINDOWS
    #include <conio.h>
#endif

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

unsigned char * Memory;

// -----------------------------------------------

char Carry = 0;

// -----------------------------------------------

char Negative = 0;

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

char * FileName;

// -----------------------------------------------

void *Commands[0x100];

// -----------------------------------------------

int ArgsCounter = 0;

// -----------------------------------------------

void * FileStreamMapping[0x100];

// -----------------------------------------------

#pragma endregion get/set

// -----------------------------------------------

#pragma region Formats

// -----------------------------------------------

int Format3(unsigned char one, unsigned char two, unsigned char three)
{
    Condition = (one >> 4) & 0xff;
    A =  one & 0x0f;
    B = 0;
    C =  (two << 8) | three;

    return 1;
}

int Format2(unsigned char one, unsigned char two, unsigned char three)
{
    Condition = (one >> 4) & 0xff;
    A =  one & 0x0f;
    B = ( two & 0xf0) >> 4;
    C =  ((two & 0x0F) << 8) | three;

    return 1;
}

int Format1(unsigned char one, unsigned char two, unsigned char three)
{
    Condition = (one >> 4) & 0xff;
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

    //printf("r0=%x,r1=%xr2=%x,r3=%x,r4=%x,r9=%x,r10=%xr11=%x,r12=%x,sp=%x,lr=%x,pc=%x,c=%x,a=%x,b=%x,c,%x,con=%x,carray=%x,zero=%x\n",Registers[0],Registers[1],Registers[2],Registers[3],Registers[4],Registers[9],Registers[10],Registers[11],Registers[12],Registers[13], Registers[14], Registers[15],Command, A,B,C, Condition, Carry, Zero);

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

int MakeArguments ( int args_Length_int,char *args_chars[] )
{
    unsigned int length = Registers[12];
    unsigned int adresseArgumentsArray = length;
    Registers[1] = adresseArgumentsArray;
    unsigned int argumentAdresse = adresseArgumentsArray + 4 + ArgsCounter * 4;
    (*(int *)(Memory + adresseArgumentsArray)) = ArgsCounter << 2;
    //adresseArgumentsArray += 4;

    Registers[12] = argumentAdresse;
    if (ArgsCounter == 0) return 1;

    int isfile = 0;

    for (int i = 1; i < args_Length_int; i++)
    {
        if (isfile)
        {
            adresseArgumentsArray += 4;
            (*(int *)(Memory + adresseArgumentsArray)) = argumentAdresse;

            unsigned int size = strlen(args_chars[i]);

            (*(int *)(Memory + argumentAdresse)) = size;

            argumentAdresse += 4;

            memcpy(((char*)(Memory + argumentAdresse)), args_chars[i], size);

            unsigned int test = size & 0x3;
            if (test != 0) size = (size ^ test) + 4;
            argumentAdresse += size;

            continue;
        }
        isfile = 1;
    }

    Registers[12] = argumentAdresse;

    return 1;
}

// -----------------------------------------------

int ReadFile ( int args_Length_int,char *args_chars[] )
{
    fseek(fd_int, 0L, SEEK_END);
    unsigned int filesize = ftell(fd_int);
    rewind(fd_int);

    Registers[15] = 0;
    Registers[13] = MemorySize - 4;
    Registers[12] = filesize;

    if ( 1 != fread( Memory, filesize, 1, fd_int ) ) printf ("can not open yexe"),exit(10);

    fclose (fd_int);

    MakeArguments ( args_Length_int, args_chars );

    return 1;
}

// -----------------------------------------------

#pragma region Memory

// -----------------------------------------------

char* GetStringFromRegister(int reg)
{
    unsigned int adresse = Registers[reg];

    unsigned int * length = (unsigned int*) (adresse + Memory);

    char * result = malloc((*length) + 1);
    result[*length] = 0;

    memcpy(result, ((char *)Memory) + adresse + 4, *length);

    return result;
}

// -----------------------------------------------

int CreateBlock(unsigned int adress, unsigned int blockState, unsigned int size)
{

    (*(int *)(Memory + adress)) = blockState;

    (*(int *)(Memory + adress + 4)) = size;

    return 1;
}

// -----------------------------------------------

unsigned int ReservedNewBlock(unsigned int currentadress, unsigned int size, unsigned int nextBlockState, unsigned int currentBlockSize)
{
    CreateBlock ( currentadress, 2, size );

    currentadress = currentadress + 8;

    if (nextBlockState == 0) return currentadress;

    unsigned int nextAdress = currentadress + size;

    size = size + 8;

    size = currentBlockSize - size;

    CreateBlock ( nextAdress, 1, size );

    return currentadress;
}

// -----------------------------------------------

unsigned int Malloc(int size)
{
    int test = size & 0x3;
    if (test) size = (size ^ test) + 4;

    unsigned int start = Registers[0];

    unsigned int currentadress;
    int currentBlockState;
    unsigned int currentBlockSize;
    unsigned nextAdress = start;

    while ( 1 )
    {
        currentadress = nextAdress;

        currentBlockSize = *(unsigned int *)(Memory + currentadress + 4);

        nextAdress = currentadress + currentBlockSize + 8;

        if (Memory[currentadress] == 2) continue;

        if (size <= currentBlockSize) break;

        if (Memory[currentadress] == 1)
        {
            printf("out of memory");
            exit (3);
            return 0;
        }
    }

    currentBlockState = Memory[currentadress];

    return ReservedNewBlock ( currentadress, size, currentBlockState, (int)currentBlockSize );
}

// -----------------------------------------------

#pragma endregion Memory

// -----------------------------------------------

#pragma region IOMapper

// -----------------------------------------------

void IsFileExist()
{
    char * path = GetStringFromRegister(2);

    unsigned int result = 0;
    if (access( path, F_OK ) == 0) result = 0xff;

    Registers[12] = result;

    free (path);
}

// -----------------------------------------------

int ReadData()
{
    char * path = GetStringFromRegister(2);

    FILE * fp = fopen(path, "rb");
    if (!fp)
    {
        printf("can not read file");
        exit(2);
    }

    fseek(fd_int, 0L, SEEK_END);
    unsigned int filesize = ftell(fd_int);
    rewind(fd_int);

    unsigned int adresse = Malloc(filesize + 4);

    (*(unsigned int *)(adresse + Memory)) = filesize;

    char * dataTarget = (char *)(adresse + Memory + 4);

    fread(dataTarget, 1, filesize, fp);

    Registers[12] = adresse;

    fclose(fp);

    free (path);

    return 1;
}

// -----------------------------------------------

int ReadObject()
{
    char * path = GetStringFromRegister(2);

    FILE * fp = fopen(path, "rb");
    if (!fp)
    {
        printf("can not read file");
        exit(2);
    }

    fseek(fd_int, 0L, SEEK_END);
    unsigned int filesize = ftell(fd_int);
    rewind(fd_int);

    unsigned int adresse = Malloc(filesize);

    char * dataTarget = (char *)(adresse + Memory);

    fread(dataTarget, 1, filesize, fp);

    Registers[12] = adresse;

    fclose(fp);

    free (path);

    return 1;
}

// -----------------------------------------------

int WriteData(unsigned int adresse)
{
    char * path = GetStringFromRegister(2);

    unsigned int * length = (unsigned int*) (adresse + Memory);

    char * data = (char*) (adresse + Memory + 4);

    FILE * fp = fopen(path, "w+");
    if (!fp)
    {
        printf("can not write file");
        exit(4);
    }

    fwrite(data, 1, *length, fp);

    fclose(fp);

    free (path);

    return 1;
}

// -----------------------------------------------

int OpenReadStream()
{
    char * path = GetStringFromRegister(2);

    FILE * fp = fopen(path, "rb");

    if (fp == 0) return 0;

    for (int i = 0; i < 0x100; i++)
    {
        if ( FileStreamMapping[i] != 0) continue;

        FileStreamMapping[i] = (void *)fp;

        Registers[12] = i;

        return 1;
    }

    printf("maximum of file streams open");
    exit(1);

    return 1;
}

// -----------------------------------------------

int OpenWriteStream()
{
    char * path = GetStringFromRegister(2);

    FILE * fp = fopen(path, "wb");

    if (fp == 0) return 0;

    for (int i = 0; i < 0x100; i++)
    {
        if ( FileStreamMapping[i] != 0) continue;

        FileStreamMapping[i] = (void *)fp;

        Registers[12] = i;

        return 1;
    }

    printf("maximum of file streams open");
    exit(1);

    return 1;
}

// -----------------------------------------------

int ReadStream ()
{
    FILE * fp = (FILE *) FileStreamMapping[Registers[2]];
    unsigned int size = Registers[3];

    char * tmp = malloc(size);

    unsigned int length = fread(tmp, 1, size, fp);

    if (!length)
    {
        free (tmp);

        Registers[12] = 0;

        return 1;
    }

    unsigned int adresse = Malloc(length + 4);

    (*(unsigned int *)(adresse + Memory)) = length;

    char * dataTarget = (char *)(adresse + Memory + 4);

    memcpy(dataTarget, tmp, length);

    free (tmp);

    Registers[12] = adresse;

    return 1;
}

// -----------------------------------------------

int WriteStream ()
{
    FILE * fp = (FILE *) FileStreamMapping[Registers[2]];
    unsigned int adresse = Registers[3];

    unsigned int * length = (unsigned int*) (adresse + Memory);

    char * data = (char*) (adresse + Memory + 4);

    fwrite(data, 1, *length, fp);

    return 1;
}

// -----------------------------------------------

int CloseReadStream()
{
    FILE * fp = (FILE *) FileStreamMapping[Registers[2]];

    fclose(fp);

    FileStreamMapping[Registers[2]] = 0;

    return 1;
}

// -----------------------------------------------

#pragma endregion IOMapper

// -----------------------------------------------

void SetCursorPosition(int xpos, int ypos)
{
    printf ( "\x1B[%d;%dH", ypos + 1, xpos + 1 );
}

// -----------------------------------------------

void SetBackgroundColor(int color)
{
    printf ( "\x1B[48;5;%dm", color );
}

// -----------------------------------------------

void SetForegroundColor(int color)
{
    printf ( "\x1B[38;5;%dm", color );
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

void AndImediateCommand()
{
    Registers[A] = Registers[B] & C;
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
    long cmpresult = ((long)Registers[A]) - ((long)C);

    Carry = cmpresult < 0 ? 1 : 0;
    Zero = cmpresult == 0 ? 1 : 0;
    //TODO:
    //Negative = 
}

// -----------------------------------------------

void CmpRegisterCommand()
{
    long cmpresult = ((long)Registers[A]) - ((long)Registers[B]);

    Carry = cmpresult < 0 ? 1 : 0;
    Zero = cmpresult == 0 ? 1 : 0;
    //TODO:
    //Negative = 
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

        read -= 1;

        unsigned int adresse = Malloc(read + 4);

        (*(unsigned int *)(adresse + Memory)) = read;

        char * dataTarget = (char *)(adresse + Memory + 4);

        memcpy(dataTarget, line, read);

        Registers[12] = adresse;

        free (line);
    }

    if (subcmd == 6)
    {
        if (Registers[1] == 1)
        {
            WriteData(Registers[3] - 4);
            return;
        }
        if (Registers[1] == 2)
        {
            WriteData(Registers[3]);
            return;
        }
        if (Registers[1] == 3)
        {
            ReadObject();
            return;
        }
        if (Registers[1] == 4)
        {
            ReadData();
            return;
        }

        if (Registers[1] == 5)
        {
            IsFileExist();

            return;
        }

        if (Registers[1] == 6)
        {
            OpenReadStream();

            return;
        }

        if (Registers[1] == 7)
        {
            ReadStream();

            return;
        }

        if (Registers[1] == 8)
        {
            CloseReadStream();

            return;
        }

        if (Registers[1] == 9)
        {
            OpenWriteStream();

            return;
        }

        if (Registers[1] == 10)
        {
            WriteStream();

            return;
        }
    }

    if (subcmd == 7)
    {
        if (Registers[1] == 1)
        {
            SetCursorPosition ( Registers[2], Registers[3] );

            return;
        }
        if (Registers[1] == 2)
        {
            SetForegroundColor ( Registers[2] );

            return;
        }
        if (Registers[1] == 3)
        {
            SetBackgroundColor ( Registers[2] );

            return;
        }
        if (Registers[1] == 4)
        {

#ifdef WINDOWS
            Registers[12] = getch();
#else
            Registers[12] = getchar();
#endif

            return;
        }
    }
}

// -----------------------------------------------

void LdrCommand()
{
    unsigned int adresse = Registers[B] + (C << 2);

    if (adresse > MemorySize)
    {
        printf("out of bounds");
        exit (1);
    }
    unsigned int * data = (unsigned int*) (adresse + Memory);

    Registers[A] = *data;
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

        unsigned int * data = (unsigned int*) (adresse + Memory);

        Registers[i] = *data;
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

        unsigned int * dataTarget = (unsigned int*) (adresse + Memory);

        (*dataTarget) = data;

        Registers[13] -= 4;
    }

}

// -----------------------------------------------

void StrCommand()
{
    unsigned int adresse = Registers[B] + (C << 2);

    unsigned int data = Registers[A];

    unsigned int * dataTarget = (unsigned int*) (adresse + Memory);

    (*dataTarget) = data;
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

void MovStatusRegisterToGenericCommand()
{
    Registers[A] = Negative << 3 | Zero << 2 | Carry << 1;
}

// -----------------------------------------------

#pragma endregion Commands

// -----------------------------------------------

#pragma region main

// -----------------------------------------------

int InitCommands()
{
    for (int i = 0; i < 0x100; i++)
    {
        FileStreamMapping[i] = 0;
    }

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
    Commands[0x16] = &AndImediateCommand;
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
    Commands[0x42] = &MovStatusRegisterToGenericCommand;
    Commands[0x41] = &PopCommand;
    Commands[0x40] = &PushCommand;
    Commands[0x1C] = &StrCommand;
    Commands[0x12] = &SubImediateCommand;
    Commands[0x52] = &SubRegisterCommand;

    return 1;
}

// -----------------------------------------------

int ParseCommandArguments(int args_Length_int,char *args_chars[])
{
    int isfile = 0;

    for (int i = 1; i < args_Length_int; i++)
    {
        if (strcmp(args_chars[i], "help") == 0)
        {
            printf ("./yamaRuntime.exe out.yexe\n");
            printf ("size {size}   The Size of memory in bytes / decimal format\n");
            printf ("{file}        A .yexe file to execute\n");

            exit(1);
        }
        if (strcmp(args_chars[i], "size") == 0)
        {
            i += 1;

            char *end;
            char *str = args_chars[i];
            long int result = strtol(str, &end, 10);

            if (end == str) printf("can not parse memory input"), exit(1);

            MemorySize = result;

            continue;
        }
        if (isfile)
        {
            ArgsCounter += 1;

            continue;
        }

        FileName = args_chars[i];
        isfile = 1;
    }

    return isfile;
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
    if (!ParseCommandArguments(args_Length_int, args_chars)) printf("try help"), exit(-1);

    Memory = malloc(MemorySize);

    InitCommands();

    fd_int = fopen(FileName,"rb");//open(args_chars[1], O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR);

    if( !fd_int ) printf("file not found"),exit(1);

    ReadFile(args_Length_int, args_chars);

    return Run ();
}

// -----------------------------------------------

#pragma endregion main

// -----------------------------------------------