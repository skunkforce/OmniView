import os
import shutil
import pandas as pd
from datetime import datetime

def process_csv(input_filename, output_filename, original_sampling_rate, target_sampling_rate):
    # Datei einlesen
    with open(input_filename, 'r') as file:
        lines = file.readlines()

    # Channel A Werte extrahieren
    channel_a_values = list(map(float, lines[3].split()))

    # Berechne den Downsampling-Faktor
    downsampling_factor = original_sampling_rate // target_sampling_rate

    # Downsampling der Channel A Werte
    downsampled_channel_a_values = channel_a_values[::downsampling_factor]

    # Zeitspalte basierend auf der Ziel-Sampling-Rate berechnen
    time_values = [(i / target_sampling_rate) * 1000 for i in range(len(downsampled_channel_a_values))]

    # Ein DataFrame erstellen
    df = pd.DataFrame({
        'Time (ms)': time_values,
        'Channel A (V)': downsampled_channel_a_values
    })

    # Multi-Header
    multi_header = "Time,Channel A\n(ms),(V)\n"

    # DataFrame in eine CSV-Datei speichern, aber ohne Header
    df.to_csv(output_filename, index=False, header=False)

    # Multi-Header manuell hinzufügen
    with open(output_filename, 'r') as file:
        data = file.read()

    with open(output_filename, 'w') as file:
        file.write(multi_header + data)

    print(f"CSV-Datei erfolgreich erstellt und gespeichert: {output_filename}")

def main():
    # Verzeichnis für die Eingabedateien
    base_directory_path = '../saves'
    originals_directory_path = os.path.join(base_directory_path, 'originals')

    # Erstelle den Ordner 'originals', falls er nicht existiert
    if not os.path.exists(originals_directory_path):
        os.makedirs(originals_directory_path)

    # Hole das aktuelle Datum und die Uhrzeit
    now = datetime.now()
    date_str = now.strftime('%Y-%m-%d_%H-%M')
    
    # Erstelle einen Unterordner mit Datum und Minutenangabe innerhalb des 'originals' Ordners
    date_time_subdirectory_path = os.path.join(originals_directory_path, date_str)
    
    if not os.path.exists(date_time_subdirectory_path):
        os.makedirs(date_time_subdirectory_path)

    # Liste aller CSV-Dateien im Verzeichnis
    csv_files = [f for f in os.listdir(base_directory_path) if f.endswith('.csv')]

    # Durchlaufe alle CSV-Dateien und verarbeite sie
    for file_name in csv_files:
        input_filename = os.path.join(base_directory_path, file_name)
        output_filename = os.path.join(base_directory_path, f'processed_{file_name}')
        
        # Beispielwerte für Sampling-Raten, ersetze sie durch die tatsächlichen Werte
        original_sampling_rate = 100000
        target_sampling_rate = 3000

        process_csv(input_filename, output_filename, original_sampling_rate, target_sampling_rate)

        # Verschiebe die Originaldatei in den Unterordner mit Datum und Minutenangabe
        shutil.move(input_filename, os.path.join(date_time_subdirectory_path, file_name))

if __name__ == "__main__":
    main()
