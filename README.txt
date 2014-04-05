todo
-need a way to store struct sockaddr_in
linked-list? or others?
addrList {
	sockaddr_in addr;
	int id; //has to be unique
	char[MAXNAME] name;
	addrList next;
} //for linkedlist

addrList alist;
addrListInit(alist); //init
newid = addrListAdd(alist, newAddr); //add and return new id
addrListDel(alist, id, name); //delete

-----
next todo

*multicast the server msg from keyboard
*get rid of g_addr, make it local