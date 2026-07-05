#include <FS.h>

void listDirectory(fs::FS &fs, const char *path)
{
    File dir = fs.open(path);

    if (!dir || !dir.isDirectory())
    {
        Serial.println("Erreur");
        return;
    }

    File entry = dir.openNextFile();

    while (entry)
    {
        if (entry.isDirectory())
        {
            Serial.printf("[DIR ] %s\n", entry.name());
        }
        else
        {
            Serial.printf("[FILE] %s (%d octets)\n",
                          entry.name(),
                          entry.size());
        }

        entry = dir.openNextFile();
    }
}