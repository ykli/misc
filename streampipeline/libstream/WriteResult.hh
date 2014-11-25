#ifndef __WRITERESULT_H__
#define __WRITERESULT_H__

class WriteResult {
public:
	void WriteFd(char *ibuf);
	WriteResult(char *ret_store_path);
	~WriteResult();
private:
	int fd;
	char *path;
};
#endif
