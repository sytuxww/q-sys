#ifndef QSYS_PIN_YIN_H
#define QSYS_PIN_YIN_H

//��ѯ��������
typedef const struct{//���ڵڶ��β�� �����β���һ�β��Ч��Ҫ��
	const unsigned char *matchstr;//�ڶ����ַ���ʼ��ƥ���ַ�����
	const unsigned char *pmaybe;//ָ����Ӧ�������ַ��������ָ��
}PY_indexdef;
 
//ƴ����ѯ���� 
//����:ƴ���ַ��� 
//���:�������һ��ƥ��ɹ������ƺ������� ������һ����ĸ����ƥ�����ѯʧ�ܷ���NULL
extern const unsigned char * py_to_str(unsigned char input_py_val[]);

#endif

