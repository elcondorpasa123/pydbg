// ���� ifdef ���Ǵ���ʹ�� DLL �������򵥵�
// ��ı�׼�������� DLL �е������ļ��������������϶���� PYDBG_EXPORTS
// ���ű���ġ���ʹ�ô� DLL ��
// �κ�������Ŀ�ϲ�Ӧ����˷��š�������Դ�ļ��а������ļ����κ�������Ŀ���Ὣ
// PYDBG_API ������Ϊ�Ǵ� DLL ����ģ����� DLL ���ô˺궨���
// ������Ϊ�Ǳ������ġ�
#ifdef PYDBG_EXPORTS
#define PYDBG_API __declspec(dllexport)
#else
#define PYDBG_API __declspec(dllimport)
#endif

// �����Ǵ� PyDbg.dll ������
class PYDBG_API CPyDbg {
public:
	CPyDbg(void);
	// TODO: �ڴ�������ķ�����
};

extern PYDBG_API int nPyDbg;

PYDBG_API int fnPyDbg(void);
