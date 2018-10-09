#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mytar.h"

extern char *use;

/** Loads a string from a file.
 *
 * file: pointer to the FILE descriptor
 *
 * The loadstr() function must allocate memory from the heap to store
 * the contents of the string read from the FILE.
 * Once the string has been properly built in memory, the function returns
 * the starting address of the string (pointer returned by malloc())
 *
 * Returns: !=NULL if success, NULL if error
 */
char*
loadstr(FILE * file)
{
    size_t size = 0;
    char c;
    while ((c = getc(file)) != '\0' && c != EOF){
        size++;
    }
    if (size == 0){
        return NULL;
    }
    if (c == EOF){
    	return NULL;
    }
    size++;
    fseek(file, -size, SEEK_CUR);
    size--;
    char *fileName = (char *)malloc(size);
    c = getc(file);
    int i = 0;
    while (c != '\0'){
    	fileName[i] = c;
    	c = getc(file);
    	i++;
    }
    return fileName;
}

/** Copy nBytes bytes from the origin file to the destination file.
 *
 * origin: pointer to the FILE descriptor associated with the origin file
 * destination:  pointer to the FILE descriptor associated with the destination file
 * nBytes: number of bytes to copy
 *
 * Returns the number of bytes actually copied or -1 if an error occured.
 */
int
copynFile(FILE * origin, FILE * destination, int nBytes)
{
    int size = 0;
    char *entireFile = (char *)malloc(nBytes);
    size = fread(entireFile, 1, nBytes, origin);
    int sizeCopied = fwrite(entireFile, 1, size, destination);
    return (sizeCopied == nBytes)?sizeCopied:-1;
}

/** Read tarball header and store it in memory.
 *
 * tarFile: pointer to the tarball's FILE descriptor
 * nFiles: output parameter. Used to return the number
 * of files stored in the tarball archive (first 4 bytes of the header).
 *
 * On success it returns the starting memory address of an array that stores
 * the (name,size) pairs read from the tar file. Upon failure, the function returns NULL.
 */
stHeaderEntry*
readHeader(FILE * tarFile, int *nFiles)
{
    if (fread(nFiles, sizeof(int), 1, tarFile) == 0){
        return NULL;
    }

    stHeaderEntry *tarHeader = (stHeaderEntry *)malloc(sizeof(stHeaderEntry) * *nFiles);
    if (tarHeader == NULL){
        return NULL;
    }
    int i;
    for (i = 0; i < *nFiles; i++){
        tarHeader[i].name = loadstr(tarFile);
        if (tarHeader[i].name == NULL){
        	return NULL;
        }
        fread(&tarHeader[i].size, sizeof(int), 1, tarFile);
    }
	return tarHeader;
}

/** Creates a tarball archive
 *
 * nfiles: number of files to be stored in the tarball
 * filenames: array with the path names of the files to be included in the tarball
 * tarname: name of the tarball archive
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE.
 * (macros defined in stdlib.h).
 *
 * HINTS: First reserve room in the file to store the tarball header.
 * Move the file's position indicator to the data section (skip the header)
 * and dump the contents of the source files (one by one) in the tarball archive.
 * At the same time, build the representation of the tarball header in memory.
 * Finally, rewind the file's position indicator, write the number of files as well as
 * the (file name,file size) pairs in the tar archive.
 *
 * Important reminder: to calculate the room needed for the header, a simple sizeof
 * of stHeaderEntry will not work. Bear in mind that, on disk, file names found in (name,size)
 * pairs occupy strlen(name)+1 bytes.
 *
 */
int
createTar(int nFiles, char *fileNames[], char tarName[])
{
    FILE *tarFile = fopen(tarName, "w");
    fwrite(&nFiles, sizeof(nFiles), 1, tarFile);
    stHeaderEntry *tarHeader = (stHeaderEntry *)malloc(sizeof(stHeaderEntry) * nFiles);
    if (tarHeader == NULL){
        return EXIT_FAILURE;
    }
    int i;
    for (i = 0; i < nFiles; i++){
        tarHeader[i].name = (char *)malloc(strlen(fileNames[i]));
        strcpy(tarHeader[i].name, fileNames[i]);
        FILE *fileRead = fopen(fileNames[i], "r");
        int sizeFile = 0;
        while (getc(fileRead) != EOF){
            sizeFile++;
        }
        tarHeader[i].size = sizeFile;
        fclose(fileRead);
    }
    for (i = 0; i < nFiles; i++){
        fwrite(tarHeader[i].name, strlen(tarHeader[i].name) + 1, 1, tarFile);
        fwrite(&tarHeader[i].size, sizeof(tarHeader[i].size), 1, tarFile);
    }
    for (i = 0; i < nFiles; i++){
        FILE *fileRead = fopen(fileNames[i], "r");
        if (copynFile(fileRead, tarFile, tarHeader[i].size) == -1){
            fclose(fileRead);
            fclose(tarFile);
            return EXIT_FAILURE;
        }
        else {
            fclose(fileRead);
        }
    }
    fclose(tarFile);
	return EXIT_SUCCESS;
}

/** Extract files stored in a tarball archive
 *
 * tarName: tarball's pathname
 *
 * On success, it returns EXIT_SUCCESS; upon error it returns EXIT_FAILURE.
 * (macros defined in stdlib.h).
 *
 * HINTS: First load the tarball's header into memory.
 * After reading the header, the file position indicator will be located at the
 * tarball's data section. By using information from the
 * header --number of files and (file name, file size) pairs--, extract files
 * stored in the data section of the tarball.
 *
 */
int
extractTar(char tarName[])
{
    FILE *tarFile = fopen(tarName, "r");
    int nFiles = 0, i;
    stHeaderEntry *tarHeader = readHeader(tarFile, &nFiles);
    if (tarHeader == NULL){
        return EXIT_FAILURE;
    }
    for (i = 0; i < nFiles; i++){
        FILE *fileToExtract = fopen(tarHeader[i].name, "w");
        if (copynFile(tarFile, fileToExtract, tarHeader[i].size) == -1){
        	fclose(fileToExtract);
            return EXIT_FAILURE;
        }
        else {
        	fclose(fileToExtract);
        }
    }
    fclose(tarFile);
	return EXIT_SUCCESS;
}
