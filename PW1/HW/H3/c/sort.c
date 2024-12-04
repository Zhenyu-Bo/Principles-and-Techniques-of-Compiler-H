#include <stdio.h>
#include <stdlib.h>
void bubbleSort(int *a, int n) {
    int i, j, temp;
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - 1 - i; j++) {
            if (a[j] > a[j + 1]) {
                temp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = temp;
            }
        }
    }
}

int main()
{
    int n;
    scanf("%d", &n);
    int *a = (int *)malloc(sizeof(int) * n);
    for(int i  = 0; i < n; i++) {
        scanf("%d", &a[i]);
    }
    bubbleSort(a, n);
    for(int i = 0; i < n; i++) {
        printf("%d ", a[i]);
    }
    free(a);
    return 0;
}
