#include <iostream>
#include <algorithm>
#include <vector>

int partition(int* arr, int low, int high)
{
	int pivot = arr[high]; // pivot
	int i = (low - 1); // Index of smaller element
	for (int j = low; j < high; j++)
	{
		if (arr[j] <= pivot)
		{
			i++; // increment index of smaller element
			std::swap(arr[i], arr[j]);
		}
	}
	std::swap(arr[i + 1], arr[high]);
	return (i + 1);
}

void quickSort(int* arr, int low, int high)
{
	if (low < high)
	{
		int pi = partition(arr, low, high);
		quickSort(arr, low, pi - 1); // Before pi
		quickSort(arr, pi + 1, high); // After pi
	}
}

void merge(int* arr, int left, int mid, int right)
{
	int n1 = mid - left + 1;
	int n2 = right - mid;
	std::vector<int> L(n1), R(n2);

	for (int i = 0; i < n1; i++)
		L[i] = arr[left + i];

	for (int i = 0; i < n2; i++)
		R[i] = arr[mid + 1 + i];

	int i = 0;
	int j = 0;
	int k = left;
	while (i < n1 && j < n2)
	{
		if (L[i] <= R[j])
		{
			arr[k] = L[i];
			i++;
		}
		else
		{
			arr[k] = R[j];
			j++;
		}
		k++;
	}

	while (i < n1)
	{
		arr[k] = L[i];
		i++;
		k++;
	}

	while (j < n2)
	{
		arr[k] = R[j];
		j++;
		k++;
	}
}

void mergeSort(int* arr, int left, int right)
{
	if (left < right)
	{
		int mid = left + (right - left) / 2; // Avoids overflow

		mergeSort(arr, left, mid);
		mergeSort(arr, mid + 1, right);
		merge(arr, left, mid, right);
	}
}

int main()
{
	// 10 integer number array for storing the numbers
	int arr[] = { 1, 5, 2, 10, 9, 8, 4, 3, 7, 6 };

	for (const auto& num : arr)
	{
		std::cout << num << " ";
	}

	std::cout << '\n';

	//quickSort(arr, 0, 9);
	mergeSort(arr, 0, 9);

	for (const auto& num : arr)
	{
		std::cout << num << " ";
	}
	return 0;
}
