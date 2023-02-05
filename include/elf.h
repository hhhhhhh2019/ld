#define TYPE_EXEC 1
#define TYPE_STAT 2
#define TYPE_DYN  3

#define SEC_TYPE_CODE 1
#define SEC_TYPE_ADDR 2
#define SEC_TYPE_NAME 3
#define SEC_TYPE_DATA 4
#define SEC_TYPE_DBUG 5


#pragma pack(push, 1)
typedef struct{
	char magic[6];
	char version;
	char type;
	// TODO: стeк

	unsigned long entry;
	unsigned long code_entry;
	unsigned long name_entry;
	unsigned long addr_entry;
} ELF;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct{
	unsigned long size;
	char data[];
} Code_sec;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct{
	char name[64];
	unsigned long offset;
} Name_sec_elem;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct{
	unsigned long size;
	Name_sec_elem elems[];
} Name_sec;
#pragma pack(pop)


#pragma pack(push, 1)
typedef struct{
	char name[64];
	unsigned long offset;
} Addr_sec_elem;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct{
	unsigned long size;
	Addr_sec_elem elems[];
} Addr_sec;
#pragma pack(pop)
