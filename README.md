#PROCESSES_DLL
Тестовое задание полученное от компании в которую устраивался. Задание ("NNSoft-Task-2020") лежит в корне в формате PDF.

Анотация: "Необходимо создать программу – аналог вкладки “Services” с графическим интерфейсом на WPF с применением паттерна MVVM. Основные функции приложения по работе со службами должны быть реализованы в native (не .NET assembly) DLL на С++. Список служб должен включать колонки Name, PID, Description, Status и Group. Необходимо также добавить колонку “Image path”, содержащую полный путь до исполняемого модуля службы, которой нет в Task Manager’e."

В этом репозитории лежит версия программы, организованная с помощью интерфейса(клиента) и dll(сервер), что отчасти соответствует условию задания по использованию паттерна.  По сравнению с версией "PROCESSES", в этой версии учтены и добавлены условия или обработчики исключений для закрытия дескрипторов процесса, остановки процесса, перезапуска процесса, происходит проверка на некоторые типы ошибок, но не все. dll написана с использованием библиотек Win32API.

Среда разработки VS2017.
