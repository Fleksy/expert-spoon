/**
 * Вариант задания 22.
 * Первая задача о Винни-Пухе, или неправильные пчелы.
 * Неправильные пчелы, подсчитав в конце месяца убытки от наличия в лесу
 * Винни-Пуха, решили разыскать его и наказать в назидание всем другим
 * любителям сладкого. Для поисков медведя они поделили лес на участки,
 * каждый из которых прочесывает одна стая неправильных пчел. В случае
 * нахождения медведя на своем участке стая проводит показательное
 * наказание и возвращается в улей. Если участок прочесан, а Винни-Пух на
 * нем не обнаружен, стая также возвращается в улей. Требуется создать
 * многопоточное приложение, моделирующее действия пчел. При решении
 * использовать парадигму портфеля задач.
 * Выполнил студент БПИ 198 Мунтян Александр.
 */
#include <math.h>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <pthread.h>
#include <ctime>
#include <random>

    using namespace std;

pthread_rwlock_t rwlock; //блокировка чтения-записи

// Лес.
char* forest;
// Текущий сектор леса.
int currentSector = 0;
// Количество секторов в лесе.
int sectors_amount;
//разыскивается ли Винни Пух сейчас или нет
bool reached = false;
//поток, который нашел Винни
int winner = -1;

// Количество потоков.
int const THREADS_AMOUNT = 4;

//метод для поиска Винни Пуха в лесу
void* threadSearch(void* param)
{
    //получение номера потока
    int currThread = *((int*)param);
    //выполянем наш поиск до тех пор пока Винни не найден
    while (!reached) {
        // Портфель задач: берем сектор из списка секторов, которые необходимо проверить.
        // Блокируем для чтения
        pthread_rwlock_wrlock(&rwlock);
        // Считываем сектор, который проверяет данный поток и сдвигаем указатель на следующий сектор.
        int i = currentSector++;
        // Получаем значение данного сектора в массиве (есть там Винни или нет) W - присутствует * - отсутсвует.
        char sector = forest[i];
        // Разблокируем для чтения.
        pthread_rwlock_unlock(&rwlock);

        //если в данном секторе присутствует Винни - пчелы его находят
        if (sector == 'W') {
            // Инфо о поимке зверя.
            fprintf(stdout, "\nПоисковый отряд под номером <%d> нашёл Винни Пуха в секторе <%d>!", currThread, i);
            // Изменение флага состояния поиска.
            reached = true;
            // Запоминаем первый нашедший отряд.
            winner = currThread;
        }
        else {
            //если винни не окозалось в секторе - выводится соответствующее предупреждение.
            fprintf(stdout,
                "Поисковой отряд пчел <%d> не нашел медведя в <%d> секторе. Поиски продолжаются.\n", currThread, i);
        }
    }
    return nullptr;
}


int main() {
    pthread_attr_t threadAttr;
    pthread_attr_init(&threadAttr);
    pthread_attr_setdetachstate(&threadAttr, PTHREAD_CREATE_DETACHED);

    cout << "Введите количество участков, на которые разбит лес: ";
    cin >> sectors_amount;
    while (sectors_amount <= 0 || sectors_amount > 1000) {
        cout << "Вы ввели неподходящее число, попробуйте еще раз (от 0 до 1000).\n";
        cin >> sectors_amount;
    }

    forest = new char [sectors_amount] {'*'};

    //Создаем гениратор и запоминаем сектор, в котором спрятался Винни Пух. 
    mt19937 generator(time(0));
    auto hereIsWinnie = uniform_int_distribution<int>(1, sectors_amount)(generator);

    //Добавляем Винни в лес. 
    forest[hereIsWinnie] = 'W';

    // Выводим сектор на экран для информирования.
    cout << "Винни пух в секторе " << hereIsWinnie << "." << endl << "Посмотрим, как быстро его смогут найти!" << endl;
    // Блокировка чтения.
    pthread_rwlock_init(&rwlock, NULL);
    // Cоздание 4 потоков.
    pthread_t threads[THREADS_AMOUNT];
    int threads_names[THREADS_AMOUNT];
    for (int i = 0; i < THREADS_AMOUNT; ++i) {
        //присваиваем номера каждому потоку
        threads_names[i] = i + 1;
        pthread_create(&threads[i], NULL, threadSearch, (void*)(threads_names + i));
    }
        // Поиск медведя.
        threadSearch((void*)&threads_names[0]);
    // Главный поток ждет выполнения всех остальных.
    for (int i = 0; i < THREADS_AMOUNT; ++i) {
        pthread_join(threads[i], NULL);
    }

    // Вывод результатов.
    cout << endl << "Поисковый отряд под номером " << winner << " нашёл Винни Пуха!" << endl;
    return 0;
}