import sqlite3
import datetime

class MyDb():

      def __init__(self):
            self.conn = sqlite3.connect("data.db")
            try:
                  self.conn.execute('''CREATE TABLE matching
                        (finger       INT    NOT NULL,
                        name          TEXT   NOT NULL,
                        room          INT    NOT NULL,
                        date          TEXT   NOT NULL);''')

                  print("Table matching not exist, create one")
                  self.commit_data()
            except:
                  print("Table matching already exist")
                  pass

            try:
                  self.conn.execute('''CREATE TABLE personal_data
                        (finger_id INT PRIMARY KEY  NOT NULL,
                        name          TEXT   NULL,
                        date          TEXT   NOT NULL);''')

                  print("Table personal_data not exist, create one")
                  self.commit_data()
            except:
                  print("Table personal_data already exist")
                  pass

      def close_db(self):
            self.conn.close()

      def commit_data(self):
            self.conn.commit()

      def save_enroll_data(self, data):
            self.conn.execute('''INSERT INTO personal_data(finger_id, name, date)
                  VALUES(?,?,?)''', (data[0], data[1], data[2]))

            self.commit_data()

      def get_enroll_data(self, id=""):
            if id == "":
                  cursor = self.conn.execute("SELECT finger_id, name, date FROM personal_data")
                  # for row in cursor:
                  #       print ("ID = ", row[0])
                  #       print ("NAME = ", row[1])
                  #       print ("ADDRESS = ", row[2], "\n")
                  return cursor.fetchall()

            elif id == 0:
                  cursor = self.conn.execute("SELECT finger_id FROM personal_data WHERE name = ?", [""])
                  return cursor.fetchall()[0]
            else:
                  cursor = self.conn.execute("SELECT name FROM personal_data WHERE finger_id = ?", [id])
                  data = cursor.fetchall()
                  # print(data)
                  if data == []:
                        print("No data based on id: " + str(id))
                        return "Unknown"
                  else:
                        name = data[0][0]
                        return name

      def save_match_data(self, data):
            retname = self.get_enroll_data(data[0])
            self.conn.execute('''INSERT INTO matching(finger,name,room,date)VALUES(?,?,?,?)''', (data[0], retname, data[1], data[2]))
            self.commit_data()
            return retname

      def get_matching_data(self, id = ""):
            cursor = self.conn.execute("SELECT finger, name, room, date from matching")
            # for row in cursor:
            #       print ("Finger ID = ", row[0])
            #       print ("Name = ", row[1])
            #       print ("Room = ", row[2])
            #       print ("Date = ", row[3], "\n")
            return cursor.fetchall()


      def delete_data(self, id):
            self.conn.execute("DELETE FROM personal_data WHERE finger_id = ?", [id])
            print("Sucessful delete id: " + str(id))
            self.commit_data()


if __name__ == "__main__":
    mydb = MyDb()
#     mydb.delete_data(1)
#     mydb.get_enroll_data()
#     data = [5, 100]
#     mydb.save_match_data(data)
#     data = mydb.get_matching_data()
#     data = [6, "", "tanggal"]
#     mydb.save_enroll_data(data)
    data = mydb.get_enroll_data()
    print(data)
#     data = mydb.get_enroll_data(0)
#     print(data[0][0])
#     if data[1][1] == None:
#           print("kososng")
#     else:
#           print("afda")