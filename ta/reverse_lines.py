lines = None
with open("LOCATED_VARIABLES.h","r") as f:
    lines = f.readlines()
    
with open("LOCATED_VARIABLES.h","w") as f:
    lines.reverse()
    f.writelines(lines)