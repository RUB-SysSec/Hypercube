import sys

if __name__ == '__main__':

    if sys.version_info.major != 3:
        print("This program requires Python 3!")
    else:
        from scripts.main import main
        main()
