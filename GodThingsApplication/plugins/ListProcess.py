def meta_data():
    res = dict()
    res["Description"] = "Python List Process"
    res["Name"] = "PyListProcess"
    res["Type"] = "Python"
    res["Class"] = "GetInfo"
    return res
import process_internal
def module_run(args):
    print("Start module1...")
    print("End module1...")
    pid_list = process_internal.get_pids()
    pids = []
    process_names = []
    for i in pid_list:
        pids.append(str(i))
        process_names.append(process_internal.get_process_name(i))
    return {"pid":pids,"process_name":process_names}

