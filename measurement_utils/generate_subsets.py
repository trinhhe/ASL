"""
Generate data from Data/ml-25m.zip

"""
import sys
import shutil

# user_steps = 3240 # ~162000 users / 3240 = 50 rating files
user_steps = 16200 # 10 files

def main():   
    print("Start Process\n")
    file = open("data/ratings_all.csv", 'r').readlines()
    users = 0
    length = len(file)
    for i in range(length):
        if i!= 0 and int(file[i].split(',')[0]) != users:
            users+=1
            if(users % user_steps == 0):
                to_write = open("data/ratings_" + str(users) +"user.csv", 'w')
                to_write.writelines(file[0:i])

    

    # shutil.copyfile(file, "data/xd.csv")
    print("End Process\n")

if __name__ == "__main__":
    main()