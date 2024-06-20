import os
import pandas as pd

# Pfad zum Verzeichnis
directory_path = '../saves'

# Liste aller CSV-Dateien im Verzeichnis
csv_files = [f for f in os.listdir(directory_path) if f.endswith('.csv')]

i = 0

for file in csv_files:
    i += 1
    file_path = os.path.join(directory_path, file)
    
    # Datei einlesen
    with open(file_path, 'r') as file:
        lines = file.readlines()

    # Zeilen "Serial" und "SamplyingRate" entfernen
    filtered_lines = [line for line in lines if not line.startswith('Serial') and not line.startswith('SamplyingRate') and not line.startswith(',,')]

    # Alle Werte in eine Liste packen
    values = ' '.join(filtered_lines).split()

    # Werte in einen DataFrame umwandeln
    df = pd.DataFrame(values, columns=['Values'])

    # Werte als float konvertieren
    df['Values'] = df['Values'].astype(float)

    # Zeitwerte berechnen
    df['Time'] = df.index / 100000  # Beispiel: Annahme, dass Zeit in Sekunden berechnet wird

    # Spaltennamen anpassen
    df.columns = ['Time', 'Channel A']

    # Erste Zeile anpassen
    first_row = pd.DataFrame([['(ms)', '(V)', '(V)', '(V)', '(V)']], columns=['Time', 'Channel A', 'Channel B', 'Channel C', 'Channel D'])

    # DataFrame zusammenfügen
    df = pd.concat([first_row, df], ignore_index=True)

    # Werte in den Spalten 'Channel A' bis 'Channel D' auf 6 Nachkommastellen runden
    df[['Channel A', 'Channel B', 'Channel C', 'Channel D']] = df[['Channel A', 'Channel B', 'Channel C', 'Channel D']].round(6)

    # Fehlende Werte mit 0.0 auffüllen
    df = df.fillna(0.0)

    # Dateinamen für die neue CSV-Datei
    new_filename = f"{i}.csv"

    # Pfad für die neue Datei
    new_file_path = os.path.join(directory_path, new_filename)

    # DataFrame als CSV speichern, ohne Indexspalte und ohne wissenschaftliche Notation
    df.to_csv(new_file_path, index=False, float_format='%.6f')

    print(f"DataFrame wurde erfolgreich als '{new_filename}' gespeichert.")
