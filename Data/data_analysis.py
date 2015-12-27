import numpy

def process_userfile(dir_name, isTTT):
    target_file = dir_name + '\\'
    if isTTT:
        target_file = target_file + 'expTTT-user.txt'
    else:
        target_file = target_file + 'expNon-user.txt'
    data = numpy.loadtxt(target_file, skiprows = 1)
    nrec = data.shape[0]

    time_each = []
    time_last = -1
    for i in range(nrec):
        if time_last == -1:
            time_last = data[i, 0]
        if data[i,2] == 3:
            time_each.append(data[i, 0]-time_last)
            time_last = -1
    return (len(time_each), sum(time_each))

def process_statefile(dir_name, isTTT, robotID):
    target_file = dir_name + '\\'
    if isTTT:
        target_file = target_file + 'expTTT-state-{}.txt'.format(str(robotID))
    else:
        target_file = target_file + 'expNon-state-{}.txt'.format(str(robotID))
    data = numpy.loadtxt(target_file, skiprows = 1)
    nrec = data.shape[0]

    time_each = []
    time_first = -1
    hit_obstacle = False
    for i in range(nrec):
        if data[i, 5] != 0 and hit_obstacle == False:
            hit_obstacle = True
            time_first = i
        elif data[i, 5] == 0 and hit_obstacle == True:
            hit_obstacle = False
            time_each.append(data[i, 0] - data[time_first, 0])
    return (len(time_each), sum(time_each))
            
def process_dir_part(dir_name, robot_num, isTTT):
    results = {}
    try:
        time_hit = 0
        n_hit    = 0
        for i in range(robot_num):
            res = process_statefile(dir_name, isTTT, i)
            n_hit = n_hit + res[0]
            time_hit = time_hit + res[1]
        results['score'] = time_hit
        results['score_avr'] = time_hit / n_hit
    except FileNotFoundError:
        expName = 'TTT' if isTTT else 'Non-TTT'
        print('No {} experiment state results or results incomplete'.format(expName))
        
    try:
        res = process_userfile(dir_name, isTTT)
        n_intervene = res[0]
        time_intervene = res[1]
        results['workload'] = time_intervene
        results['efficiency'] = time_intervene/n_intervene
    except FileNotFoundError:
        expName = 'TTT' if isTTT else 'Non-TTT'
        print('No {} experiment user input records'.format(expName))

    return results
    
def process_dir(dir_name, robot_num):
    resTTT = process_dir_part(dir_name, robot_num, True)
    resNon = process_dir_part(dir_name, robot_num, False)
    return {'TTT':resTTT, 'Non':resNon}
    

    
    
