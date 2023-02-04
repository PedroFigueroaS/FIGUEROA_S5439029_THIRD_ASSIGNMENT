#include "./../include/processA_utilities.h"
#define SEM_PATH_WRITER "/sem_AOS_writer"
#define SEM_PATH_READER "/sem_AOS_reader"
int width = 80;
int height = 30;
unsigned int sizeof_dm(int rows, int cols, size_t sizeElement) // function to calculate the length of matrix as a 1D array
{
    size_t size = rows * (sizeof(void *) + (cols * sizeElement));
    return size;
}
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
            if (mat[i][j] == 1) // for each rows, each time a "1" is detected, count it
            {
                count++;
            }
        }
        if (count + 1 == r * 2) // if the count is equal to the diameter, request that row number and add it to a new array
        {
            xcent[u] = i;
            u++;
        }
    }
    // find y_center
    // the center of the rows are the mid position of the array: xcent[u/2], in that rows is looked the y coordinate of the center
    for (int j = 0; j <= num_cols; j++)
    {
        if (mat[xcent[u / 2]][j] == 1) // for each columns, each time a "1" is detected, count it
        {
            ycent[v] = j;
            // printf("row:%d\n",ycent[v]);
            v++;
        }
    }
    *px = xcent[u / 2];
    *py = ycent[v / 2]; // the center of the cols are the mid position of the array: ycent[u/2]
}

void move_circle(int cmd, int *pxx, int *pyy, int rows, int cols, int r) // function to move the circle in the windows of process A
{

    // First, clear previous circle positions
    mvaddch(circle.y, circle.x, ' ');
    mvaddch(circle.y - 1, circle.x, ' ');
    mvaddch(circle.y + 1, circle.x, ' ');
    mvaddch(circle.y, circle.x - 1, ' ');
    mvaddch(circle.y, circle.x + 1, ' ');

    // Move circle by one character based on cmd
    switch (cmd)
    {
    case KEY_LEFT:
        if (circle.x - 1 > 0)
        {
            circle.x--;
        }
        break;
    case KEY_RIGHT:
        if (circle.x + 1 < COLS - BTN_SIZE_X - 2)
        {
            circle.x++;
        }
        break;
    case KEY_UP:
        if (circle.y - 1 > 0)
        {
            circle.y--;
        }
        break;
    case KEY_DOWN:
        if (circle.y + 2 < LINES)
        {
            circle.y++;
        }
        break;
    default:
        break;
    }
    int mat[rows][cols];         // A matrix with the dimensions of the bit map is created
    memset(mat, 0, sizeof(mat)); // this matrix is initialized with 0s
    for (int xi = -r; xi <= r; xi = xi + 1)
    {
        for (int yi = -r; yi <= r; yi = yi + 1)
        {
            // If distance is smaller, point is within the circle
            if (sqrt(xi * xi + yi * yi) < r)
            {
                mat[circle.x + xi][circle.y + yi] = 1; // create the representation of the bitmap circle in the matrix
                // mat2[20 * (circle.x + xi)][20 * (circle.y + yi)] = 1;
            }
        }
    }
    int posxA, posyA;
    //
    mat_pos_calc(rows, cols, mat, r, &posxA, &posyA); // call the function to obtain the center of the previous matrix created

    *pxx = posxA;
    *pyy = posyA;
    char str2[80];
    if (posxA < 10)
    {
        sprintf(str2, "Process A matrix PX:%d%d", 0, posxA);
    }
    else
    {
        sprintf(str2, "Process A matrix PX:%d", posxA);
    }
    mvprintw(0, 1, str2);
    if (posyA < 10)
    {
        sprintf(str2, "Process A matrix PY:%d%d", 0, posyA);
    }
    else
    {
        sprintf(str2, "Process A matrix PY:%d", posyA);
    }
    mvprintw(1, 1, str2);
    refresh();
}

int main(int argc, char *argv[])
{

    //// BITMAP PART
    bmpfile_t *bmp;
    // Data type for defining pixel colors (BGRA)
    rgb_pixel_t pixel = {255, 0, 0, 0};

    /*if (argc < 3)
    {
        printf("Please specify filename and radius as arguments!");
        return EXIT_FAILURE;
    }*/
    // properties for the bitmap
    int depth = 4;
    int N = 1;
    int mat[width][height];                         // matrix A with the same size of the bitmap (80x30)
    int mat2[20 * width][20 * height];              // matrix for the shared memory with size 20 greater than the matrix A(1600x600)
    int posx = LINES / 2, posy = COLS / 2;          // initialized position of the circle: center of the window
    memset(mat, 0, sizeof(mat));                    // initialize the matrix with 0s
    int rowsA = sizeof(mat) / sizeof(mat[0]);       // obtain rows length
    int colsA = sizeof(mat[0]) / sizeof(mat[0][0]); // obtain cols length
    memset(mat2, 0, sizeof(mat2));
    int rowsM = sizeof(mat2) / sizeof(mat2[0]);
    int colsM = sizeof(mat2[0]) / sizeof(mat2[0][0]);

    // Code for drawing a centered circle of given radius...
    int radius = N * atoi(argv[2]);

    // Utility variable to avoid trigger resize event on launch
    int first_resize = TRUE;
    // SHARED MEMORY PART
    const char *shm_name = "/AOS"; // address of the shared memory
    int shm_fd;
    int *ptr; // pointer array in wherer the information is shared between process

    shm_fd = shm_open(shm_name, O_CREAT | O_RDWR, 0666); // open and create a shared memory segment
    if (shm_fd == 1)
    {
        printf("Shared memory segment failed\n");
        exit(1);
    }

    size_t sizemessage = sizeof_dm(rowsM, colsM, sizeof(double)); // compute the size of the message to share based on the size of matrix M (1600x600)

    ftruncate(shm_fd, sizemessage);                                            // resize memory region to the length of the message
    ptr = mmap(0, sizemessage, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0); // mapping of memory segment

    if (ptr == MAP_FAILED)
    {
        printf("Map failed\n");
        return 1;
    }
    // SEMAPHORE PART
    sem_t *sem_id_writer;
    sem_t *sem_id_reader;
    sem_id_writer = sem_open(SEM_PATH_WRITER, O_CREAT, 0644, 1); // create a semaphore to write
    if (sem_id_writer == (void *)-1)
    {
        perror("sem_open failure");
        exit(1);
    }

    sem_id_reader = sem_open(SEM_PATH_READER, O_CREAT, 0644, 1); // create a semaphore to read
    if (sem_id_reader == (void *)-1)
    {
        perror("sem_open failure");
        exit(1);
    }

    sem_init(sem_id_writer, 1, 1); // initialize the counter
    sem_init(sem_id_reader, 1, 0);

    // Initialize UI
    init_console_ui();

    // Infinite loop
    while (TRUE)
    {
        // initialize variables for the process
        int u = 0;
        int posxA;
        int posyA;
        int posxsh = LINES / 2;
        int posysh = COLS / 2;
        int rowsA;
        int colsA;
        int rowsSH;
        int colsSH;

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

        // Else, if user presses print button...
        if (cmd == KEY_MOUSE)
        {
            if (getmouse(&event) == OK)
            {
                if (check_button_pressed(print_btn, &event))
                {
                    bmp = bmp_create(N * width, N * height, depth); // Create a bitmap with dimensions 80x30
                    for (int x = -radius; x <= radius; x = x + 1)
                    {
                        for (int y = -radius; y <= radius; y = y + 1) // go over every position in the bitmap, within the range of the radious
                        {
                            // If distance is smaller, point is within the circle
                            if (sqrt(x * x + y * y) < radius)
                            {
                                // Color the pixel at the specified (x,y) position
                                // with the given pixel values
                                bmp_set_pixel(bmp, N * (posx + x), N * (posy + y), pixel);
                            }
                        }
                    }
                    bmp_save(bmp, argv[1]);
                    //  Free resources before termination
                    bmp_destroy(bmp);

                    mvprintw(LINES - 1, 1, "Bitmap saved");
                    refresh();
                    sleep(1);
                    for (int j = 0; j < COLS - BTN_SIZE_X - 2; j++)
                    {
                        mvaddch(LINES - 1, j, ' ');
                    }
                }
            }
        }

        // If input is an arrow key, move circle accordingly...
        mvprintw(LINES - 1, 1, "[PROCESS A] Waiting for WRITER semaphore\n"),
            sem_wait(sem_id_writer); // Decrement the semaphore counter
        mvprintw(LINES - 1, 1, "[PROCESS A] WRITER entered!\n");
        if (cmd == KEY_LEFT || cmd == KEY_RIGHT || cmd == KEY_UP || cmd == KEY_DOWN)
        {
            move_circle(cmd, &posx, &posy, width, height, radius); // move circle and return the center position
            draw_circle();
            memset(mat2, 0, sizeof(mat2));
        }

        for (int xi = -radius; xi <= radius; xi = xi + 1)
        {
            for (int yi = -radius; yi <= radius; yi = yi + 1)
            {
                // If distance is smaller, point is within the circle
                if (sqrt(xi * xi + yi * yi) < radius)
                {
                    mat2[20 * (posx + xi)][20 * (posy + yi)] = 1; // The position in the range of the radious becomes 1 in the shared memory Matrix
                }
            }
        }
        rowsSH = sizeof(mat2) / sizeof(mat2[0]); // obtain the rows and cols size of the matrix
        colsSH = sizeof(mat2[0]) / sizeof(mat2[0][0]);
        mat_pos_calc(rowsSH, colsSH, mat2, radius, &posxsh, &posysh); // compute the center of the shared memory matrix
        char str3[80];

        if (posxsh < 100)
        {
            sprintf(str3, "Shared memory PX:%d.",posxsh);
        }
        else if (posxsh>=100 && posxsh<1000)
        {
            sprintf(str3, "Shared memory PX:%d%d",0,posxsh);
        }
        else{
            sprintf(str3, "Shared memory PY:%d", posxsh);
        }
        mvprintw(2, 1, str3);

        if (posysh < 100)
        {
            sprintf(str3, "Shared memory PY:%d.",posysh);
        }
        else if (posysh>=100 && posysh<1000)
        {
            sprintf(str3, "Shared memory PY:%d%d",0,posysh);
        }
        else{
            sprintf(str3, "Shared memory PY:%d", posysh);
        }
        mvprintw(3, 1, str3);
        refresh();

        for (int i = 0; i < rowsSH; i++)
        {
            for (int j = 0; j < colsSH; j++)
            {
                ptr[u] = mat2[i][j]; // write in the pointer each position of the matrix 2, the pointer ptr resizes the information as a 1D array
                u++;
            }
        }
        mvprintw(LINES - 2, 1, "[PROCESS A] Leaving the READER semaphore\n");
        // sleep(5);
        sem_post(sem_id_reader);
        mvprintw(LINES - 2, 1, "[PROCESS A] Semaphore READER unlocked\n");
    }
    sem_close(sem_id_reader);
    sem_close(sem_id_writer);
    sem_unlink(SEM_PATH_READER);
    sem_unlink(SEM_PATH_WRITER);
    munmap(ptr, sizemessage);
    endwin();
    return 0;
}
