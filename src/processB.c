#include "./../include/processB_utilities.h"
#define SEM_PATH_WRITER "/sem_AOS_writer"
#define SEM_PATH_READER "/sem_AOS_reader"
int width = 80;
int height = 30;
void mat_pos_calc(int num_rows, int num_cols, int mat[][num_cols], int r, int *px, int *py) // function to calculate the center of a given matrix and return its coordinates to the main program
{
    int count;
    int acum = 0;
    int xcent[num_rows];
    int ycent[num_cols];
    int u = 0;
    int v = 0;
    int b;
    // i:width j:height
    for (int i = 0; i <= num_rows; i++)
    {
        count = 0;
        for (int j = 0; j <= num_cols; j++)
        {
            if (mat[i][j] == 1)
            {
                count++;
            }
        }
        if (count + 1 == r * 2)
        {
            // printf("row:%d,count:%d\n", i, count);
            xcent[u] = i;
            u++;
        }
    }
    // find y_center
    for (int j = 0; j <= num_cols; j++)
    {
        if (mat[xcent[u / 2]][j] == 1)
        {
            ycent[v] = j;
            v++;
        }
    }
    *px = xcent[u / 2];
    *py = ycent[v / 2];
}
unsigned int sizeof_dm(int rows, int cols, size_t sizeElement) // function to calculate the length of matrix as a 1D array
{
    size_t size = rows * (sizeof(void *) + (cols * sizeElement));
    return size;
}

int main(int argc, char const *argv[])
{
    // sHARED MEMORY PART
    const char *shm_name = "/AOS"; // address of the shared memory
    int shm_fd;
    int mat[width][height];            // matrix with the same size of the bitmap (80x30)
    int mat2[20 * width][20 * height]; // matrix for the shared memory with size 20 greater than the matrix A(1600x600)
    memset(mat, 0, sizeof(mat));       // initialize the matrix with 0s
    memset(mat2, 0, sizeof(mat2));
    int rowsA = sizeof(mat) / sizeof(mat[0]);       // obtain rows length
    int colsA = sizeof(mat[0]) / sizeof(mat[0][0]); // obtain rows length
    int rowsSH = sizeof(mat2) / sizeof(mat2[0]);
    int colsSH = sizeof(mat2[0]) / sizeof(mat2[0][0]);
    size_t sizemessage = sizeof_dm(rowsSH, colsSH, sizeof(double));//compute the size of the message to share based on the size of matrix2(1600x600)

    shm_fd = shm_open(shm_name, O_RDONLY, 0666);//open and create a shared memory segment
    if (shm_fd == 1)
    {
        printf("Shared memory segment failed\n");
        exit(1);
    }

    int *ptr = mmap(0, sizemessage, PROT_READ, MAP_SHARED, shm_fd, 0);//mapping of memory segment
    if (ptr == MAP_FAILED)
    {
        printf("Map failed\n");
        return 1;
    }

    // SEMAPHORE
    sem_t *sem_id_writer;
    sem_t *sem_id_reader;
    sem_id_writer = sem_open(SEM_PATH_WRITER, 0);//create a semaphore to write
    if (sem_id_writer == (void *)-1)
    {
        mvprintw(LINES - 2, 1, "sem_open failure");
        exit(1);
    }

    sem_id_reader = sem_open(SEM_PATH_READER, 0);//create a semaphore to read
    if (sem_id_reader == (void *)-1)
    {
        mvprintw(LINES - 2, 1, "sem_open failure");
        exit(1);
    }

    int radius = 5;
    int posxsh = LINES / 2;
    int posysh = COLS / 2;
    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;

    // Initialize UI
    init_console_ui();

    // Infinite loop
    while (TRUE)
    {
        mvprintw(LINES - 1, 1, "[PROCESS B] Waiting for READER semaphore\n"),
        sem_wait(sem_id_reader);//Decrement the semaphore counter
        mvprintw(LINES - 1, 1, "[PROCESS B] READER entered!\n");
        int u = 0;
        for (int i = 0; i < rowsSH; i++)
        {
            for (int j = 0; j < colsSH; j++)
            {
                mat2[i][j] = ptr[u];//From the shared memory in the 1D pointer, extract each position and store it again in the 2D matrix
                u++;
            }
            // printf("\n");
        }
        mat_pos_calc(rowsSH, colsSH, mat2, radius, &posxsh, &posysh);//Obtain the center coordinates of the matrix with dimensions (1600x600)

        // Get input in non-blocking mode
        int cmd = getch();

        // If user resizes screen, re-draw UI...
        if (cmd == KEY_RESIZE)
        {
            if (first_resize)
            {
                first_resize = FALSE;
            }
            else
            {
                reset_console_ui();
            }
        }

        else
        {
            mvaddch(posysh / 20, posxsh / 20, '0');//In the process B windows, plot the obtained position, divided by 20 to fit in the 80x30 window
            refresh();
            char str[80];
            sprintf(str, "PX:%d.", posxsh / 20);
            mvprintw(0, 1, str);
            sprintf(str, "PY:%d.", posysh / 20);
            mvprintw(1, 1, str);
            refresh();
        }
        mvprintw(LINES - 2, 1, "[PROCESS B] Leaving the WRITER semaphore\n");
        // sleep(10);
        sem_post(sem_id_writer);
        mvprintw(LINES - 2, 1, "[PROCESS B] Semaphore WRITER unlocked\n");
        //mvaddch(posxsh/20, posysh/20, '0');
        refresh();
    }
    munmap(ptr, sizemessage);
    //sem_close(sem_id_reader);
    //sem_close(sem_id_writer);
    //sem_unlink(SEM_PATH_READER);
    //sem_unlink(SEM_PATH_WRITER);
    endwin();
    return 0;
}
